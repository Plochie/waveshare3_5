use image::imageops::FilterType;
use image::RgbaImage;
use serde::Serialize;
use serde_json::Value;
use std::collections::HashSet;
use std::fs;
use std::path::PathBuf;
use tauri::{AppHandle, Manager};

// Icon conversion + storage for the Stream Deck icon pipeline (protocol §3.3).
// Icons are stored resized to a fixed size, as raw RGB565 bytes
// (`<appdata>/icons/<name>.bin`) plus a `<name>.bin.json` sidecar carrying
// {"w":_,"h":_} - the same shape the device persists on its side.

const ICON_SIZE: u32 = 64;

#[derive(Serialize)]
pub struct IconMeta {
    pub name: String,
    pub w: u32,
    pub h: u32,
    pub preview: String,
}

fn icons_dir(app: &AppHandle) -> Result<PathBuf, String> {
    let dir = app.path().app_data_dir().map_err(|e| e.to_string())?.join("icons");
    fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    Ok(dir)
}

fn sanitize_name(stem: &str) -> String {
    let cleaned: String = stem
        .to_lowercase()
        .chars()
        .map(|c| if c.is_ascii_alphanumeric() { c } else { '_' })
        .collect();
    if cleaned.is_empty() {
        "icon".to_string()
    } else {
        cleaned
    }
}

fn base64_encode(data: &[u8]) -> String {
    use base64::{engine::general_purpose, Engine as _};
    general_purpose::STANDARD.encode(data)
}

fn rgba_to_rgb565(img: &RgbaImage) -> Vec<u8> {
    let mut out = Vec::with_capacity((img.width() * img.height() * 2) as usize);
    for px in img.pixels() {
        let [r, g, b, _] = px.0;
        let v: u16 = ((r as u16 & 0xF8) << 8) | ((g as u16 & 0xFC) << 3) | (b as u16 >> 3);
        out.extend_from_slice(&v.to_le_bytes());
    }
    out
}

fn rgb565_to_rgba(bytes: &[u8], w: u32, h: u32) -> RgbaImage {
    let mut img = RgbaImage::new(w, h);
    for (i, px) in img.pixels_mut().enumerate() {
        if i * 2 + 1 >= bytes.len() {
            break;
        }
        let v = u16::from_le_bytes([bytes[i * 2], bytes[i * 2 + 1]]);
        let r = (((v >> 11) & 0x1F) * 255 / 31) as u8;
        let g = (((v >> 5) & 0x3F) * 255 / 63) as u8;
        let b = ((v & 0x1F) * 255 / 31) as u8;
        *px = image::Rgba([r, g, b, 255]);
    }
    img
}

fn encode_png_base64(img: &RgbaImage) -> Result<String, String> {
    let mut buf = Vec::new();
    image::DynamicImage::ImageRgba8(img.clone())
        .write_to(&mut std::io::Cursor::new(&mut buf), image::ImageFormat::Png)
        .map_err(|e| e.to_string())?;
    Ok(format!("data:image/png;base64,{}", base64_encode(&buf)))
}

// Plain (non-command) helpers used by ws_server.rs to push icons over the WS.

pub fn read_icon(app: &AppHandle, name: &str) -> Option<(u32, u32, Vec<u8>)> {
    let dir = icons_dir(app).ok()?;
    let bytes = fs::read(dir.join(name)).ok()?;
    let (w, h) = match fs::read_to_string(dir.join(format!("{name}.json"))) {
        Ok(text) => {
            let v: Value = serde_json::from_str(&text).ok()?;
            (
                v["w"].as_u64().unwrap_or(ICON_SIZE as u64) as u32,
                v["h"].as_u64().unwrap_or(ICON_SIZE as u64) as u32,
            )
        }
        Err(_) => (ICON_SIZE, ICON_SIZE),
    };
    Some((w, h, bytes))
}

pub fn icons_referenced(config: &Value) -> HashSet<String> {
    let mut out = HashSet::new();
    if let Some(pages) = config.get("pages").and_then(|p| p.as_array()) {
        for page in pages {
            let Some(buttons) = page.get("buttons").and_then(|b| b.as_array()) else { continue };
            for btn in buttons {
                if let Some(icon) = btn.get("icon").and_then(|i| i.as_str()) {
                    if !icon.is_empty() {
                        out.insert(icon.to_string());
                    }
                }
            }
        }
    }
    out
}

// Frontend-facing commands.

#[tauri::command]
pub async fn import_icon(app: AppHandle, path: String) -> Result<IconMeta, String> {
    let img = image::open(&path).map_err(|e| e.to_string())?;
    let resized = img.resize_exact(ICON_SIZE, ICON_SIZE, FilterType::Triangle).to_rgba8();
    let rgb565 = rgba_to_rgb565(&resized);

    let stem = std::path::Path::new(&path)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("icon");
    let name = format!("{}.bin", sanitize_name(stem));

    let dir = icons_dir(&app)?;
    fs::write(dir.join(&name), &rgb565).map_err(|e| e.to_string())?;
    fs::write(
        dir.join(format!("{name}.json")),
        serde_json::json!({ "w": ICON_SIZE, "h": ICON_SIZE }).to_string(),
    )
    .map_err(|e| e.to_string())?;

    let preview = encode_png_base64(&resized)?;
    Ok(IconMeta { name, w: ICON_SIZE, h: ICON_SIZE, preview })
}

#[tauri::command]
pub fn icon_preview(app: AppHandle, name: String) -> Result<Option<String>, String> {
    let Some((w, h, bytes)) = read_icon(&app, &name) else { return Ok(None) };
    let img = rgb565_to_rgba(&bytes, w, h);
    Ok(Some(encode_png_base64(&img)?))
}
