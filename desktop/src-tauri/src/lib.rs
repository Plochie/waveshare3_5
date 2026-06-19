mod host_actions;
mod icons;
mod mdns;
mod ws_server;

use std::fs;
use std::path::PathBuf;
use tauri::Manager;
use ws_server::SharedState;

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
            ws_server::list_devices,
            icons::import_icon,
            icons::icon_preview
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
