mod host_actions;
mod icons;
mod mdns;
mod pairing;
mod ws_server;

use serde_json::{json, Value};
use std::fs;
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};
use tauri::Manager;
use ws_server::{PairDecision, SharedState};

const CONFIG_FILE_NAME: &str = "deck-config.json";

fn config_path(app: &tauri::AppHandle) -> Result<PathBuf, String> {
    let dir = app.path().app_data_dir().map_err(|e| e.to_string())?;
    fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    Ok(dir.join(CONFIG_FILE_NAME))
}

fn read_config_value(app: &tauri::AppHandle) -> Option<serde_json::Value> {
    let path = config_path(app).ok()?;
    let text = fs::read_to_string(path).ok()?;
    serde_json::from_str(&text).ok()
}

fn unix_now() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_secs())
        .unwrap_or(0)
}

// Writes `value` to disk and updates the in-memory canonical config.
async fn persist_config(app: &tauri::AppHandle, state: &SharedState, value: Value) -> Result<(), String> {
    let path = config_path(app)?;
    let text = serde_json::to_string_pretty(&value).map_err(|e| e.to_string())?;
    fs::write(&path, text).map_err(|e| e.to_string())?;
    *state.config.lock().await = Some(value);
    Ok(())
}

#[tauri::command]
fn load_config(app: tauri::AppHandle) -> Result<Option<String>, String> {
    let path = config_path(&app)?;
    if !path.exists() {
        return Ok(None);
    }
    fs::read_to_string(&path).map(Some).map_err(|e| e.to_string())
}

#[tauri::command]
async fn save_config(
    app: tauri::AppHandle,
    state: tauri::State<'_, SharedState>,
    json: String,
) -> Result<(), String> {
    let path = config_path(&app)?;
    fs::write(&path, &json).map_err(|e| e.to_string())?;
    let value: serde_json::Value = serde_json::from_str(&json).map_err(|e| e.to_string())?;
    ws_server::broadcast_config(&state, value).await;
    Ok(())
}

#[tauri::command]
fn config_path_string(app: tauri::AppHandle) -> Result<String, String> {
    Ok(config_path(&app)?.to_string_lossy().into_owned())
}

#[tauri::command]
async fn confirm_pairing(
    app: tauri::AppHandle,
    state: tauri::State<'_, SharedState>,
    device_id: String,
    name: String,
) -> Result<(), String> {
    let token = pairing::mint_token();
    let base = state.config.lock().await.clone().unwrap_or_else(|| json!({}));
    let next = pairing::upsert_device(&base, &device_id, &name, &token, unix_now());
    persist_config(&app, &state, next).await?;
    ws_server::resolve_pair(&state, &device_id, PairDecision::Approve(token)).await;
    Ok(())
}

#[tauri::command]
async fn reject_pairing(
    state: tauri::State<'_, SharedState>,
    device_id: String,
) -> Result<(), String> {
    ws_server::resolve_pair(&state, &device_id, PairDecision::Reject).await;
    Ok(())
}

#[tauri::command]
async fn forget_device(
    app: tauri::AppHandle,
    state: tauri::State<'_, SharedState>,
    device_id: String,
) -> Result<(), String> {
    let base = state.config.lock().await.clone().unwrap_or_else(|| json!({}));
    let next = pairing::remove_device(&base, &device_id);
    persist_config(&app, &state, next).await?;
    ws_server::disconnect_device(&state, &device_id).await;
    Ok(())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_dialog::init())
        .setup(|app| {
            let handle = app.handle().clone();
            let state: SharedState = std::sync::Arc::new(ws_server::ServerState::new(handle.clone()));
            if let Some(initial) = read_config_value(&handle) {
                let state_for_seed = state.clone();
                tauri::async_runtime::spawn(async move {
                    *state_for_seed.config.lock().await = Some(initial);
                });
            }
            app.manage(state.clone());
            tauri::async_runtime::spawn(ws_server::run(handle, state));
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            load_config,
            save_config,
            config_path_string,
            confirm_pairing,
            reject_pairing,
            forget_device,
            ws_server::list_devices,
            icons::import_icon,
            icons::icon_preview
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
