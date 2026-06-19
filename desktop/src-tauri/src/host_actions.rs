use enigo::{
    Direction::{Click, Press, Release},
    Enigo, Key, Keyboard, Settings,
};
use serde_json::Value;
use std::process::Command;

// Executes a single host step (protocol §3 "Host (via agent)" types: hotkey,
// type_text, launch_app, run_command, media_key) requested by the device over
// the exec/result WS exchange (protocol §3.4). Called from a blocking task
// (see ws_server.rs) since enigo + process spawning can block briefly.
pub fn execute_step(step: &Value) -> Result<(), String> {
    let t = step.get("type").and_then(|v| v.as_str()).unwrap_or("");
    match t {
        "hotkey" => hotkey(step),
        "type_text" => type_text(step),
        "launch_app" => launch_app(step),
        "run_command" => run_command(step),
        "media_key" => media_key(step),
        other => Err(format!("unsupported host step type: {other}")),
    }
}

fn modifier_key(name: &str) -> Option<Key> {
    match name.to_lowercase().as_str() {
        "cmd" | "command" | "meta" | "win" | "super" => Some(Key::Meta),
        "ctrl" | "control" => Some(Key::Control),
        "alt" | "option" => Some(Key::Alt),
        "shift" => Some(Key::Shift),
        _ => None,
    }
}

fn named_key(name: &str) -> Option<Key> {
    match name.to_lowercase().as_str() {
        "tab" => Some(Key::Tab),
        "esc" | "escape" => Some(Key::Escape),
        "enter" | "return" => Some(Key::Return),
        "space" => Some(Key::Space),
        "backspace" => Some(Key::Backspace),
        "delete" => Some(Key::Delete),
        "up" => Some(Key::UpArrow),
        "down" => Some(Key::DownArrow),
        "left" => Some(Key::LeftArrow),
        "right" => Some(Key::RightArrow),
        _ => None,
    }
}

// `keys` (protocol example: ["cmd","shift","a"]) is modifiers-then-main-key in
// any order; every recognized modifier is pressed, the first non-modifier is
// clicked, then modifiers are released in reverse order.
fn hotkey(step: &Value) -> Result<(), String> {
    let keys: Vec<String> = step
        .get("keys")
        .and_then(|v| v.as_array())
        .map(|a| a.iter().filter_map(|v| v.as_str().map(String::from)).collect())
        .unwrap_or_default();
    if keys.is_empty() {
        return Err("hotkey: no keys given".to_string());
    }

    let mut enigo = Enigo::new(&Settings::default()).map_err(|e| e.to_string())?;
    let mut pressed: Vec<Key> = Vec::new();
    let mut main: Option<Key> = None;
    let mut bad_key: Option<String> = None;

    for k in &keys {
        if let Some(m) = modifier_key(k) {
            if enigo.key(m, Press).is_ok() {
                pressed.push(m);
            }
        } else if let Some(n) = named_key(k) {
            main = Some(n);
        } else if k.chars().count() == 1 {
            main = Some(Key::Unicode(k.chars().next().unwrap()));
        } else {
            bad_key = Some(k.clone());
            break;
        }
    }

    let result = match bad_key {
        Some(k) => Err(format!("hotkey: unrecognized key '{k}'")),
        None => match main {
            Some(m) => enigo.key(m, Click).map_err(|e| e.to_string()),
            None => Ok(()), // modifiers only - nothing to click
        },
    };

    for m in pressed.iter().rev() {
        let _ = enigo.key(*m, Release);
    }
    result
}

fn type_text(step: &Value) -> Result<(), String> {
    let text = step.get("text").and_then(|v| v.as_str()).unwrap_or("");
    let mut enigo = Enigo::new(&Settings::default()).map_err(|e| e.to_string())?;
    enigo.text(text).map_err(|e| e.to_string())
}

fn media_key(step: &Value) -> Result<(), String> {
    let name = step.get("key").and_then(|v| v.as_str()).unwrap_or("");
    let key = match name {
        "play_pause" => Key::MediaPlayPause,
        "next" => Key::MediaNextTrack,
        "prev" => Key::MediaPrevTrack,
        "vol_up" => Key::VolumeUp,
        "vol_down" => Key::VolumeDown,
        "mute" => Key::VolumeMute,
        other => return Err(format!("media_key: unknown key '{other}'")),
    };
    let mut enigo = Enigo::new(&Settings::default()).map_err(|e| e.to_string())?;
    enigo.key(key, Click).map_err(|e| e.to_string())
}

fn launch_app(step: &Value) -> Result<(), String> {
    let target = step.get("target").and_then(|v| v.as_str()).unwrap_or("");
    if target.is_empty() {
        return Err("launch_app: no target given".to_string());
    }

    #[cfg(target_os = "macos")]
    let result = Command::new("open").arg("-a").arg(target).spawn();
    #[cfg(target_os = "windows")]
    let result = Command::new("cmd").args(["/C", "start", "", target]).spawn();
    #[cfg(all(unix, not(target_os = "macos")))]
    let result = Command::new(target).spawn();

    result.map(|_| ()).map_err(|e| e.to_string())
}

fn run_command(step: &Value) -> Result<(), String> {
    let command = step.get("command").and_then(|v| v.as_str()).unwrap_or("");
    if command.is_empty() {
        return Err("run_command: no command given".to_string());
    }

    #[cfg(target_os = "windows")]
    let result = Command::new("cmd").args(["/C", command]).spawn();
    #[cfg(not(target_os = "windows"))]
    let result = Command::new("sh").arg("-c").arg(command).spawn();

    result.map(|_| ()).map_err(|e| e.to_string())
}
