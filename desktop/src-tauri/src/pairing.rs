use serde_json::{json, Value};

// 16 random bytes hex-encoded -> 32 lowercase hex chars.
pub fn mint_token() -> String {
    let mut buf = [0u8; 16];
    getrandom::getrandom(&mut buf).expect("getrandom failed");
    buf.iter().map(|b| format!("{:02x}", b)).collect()
}

// The stored token for `device_id`, if it is in `agent.devices`.
pub fn find_paired_token(config: &Value, device_id: &str) -> Option<String> {
    config
        .get("agent")?
        .get("devices")?
        .as_array()?
        .iter()
        .find(|d| d.get("device_id").and_then(|v| v.as_str()) == Some(device_id))
        .and_then(|d| d.get("token").and_then(|v| v.as_str()).map(String::from))
}

// Inserts or replaces the registry entry for `device_id`, returning a new config.
pub fn upsert_device(config: &Value, device_id: &str, name: &str, token: &str, paired_at: u64) -> Value {
    let mut cfg = config.clone();
    if !cfg.get("agent").map_or(false, |a| a.is_object()) {
        cfg["agent"] = json!({});
    }
    if !cfg["agent"].get("devices").map_or(false, |d| d.is_array()) {
        cfg["agent"]["devices"] = json!([]);
    }
    let entry = json!({
        "device_id": device_id, "name": name, "token": token, "paired_at": paired_at
    });
    let arr = cfg["agent"]["devices"].as_array_mut().unwrap();
    match arr
        .iter()
        .position(|d| d.get("device_id").and_then(|v| v.as_str()) == Some(device_id))
    {
        Some(pos) => arr[pos] = entry,
        None => arr.push(entry),
    }
    cfg
}

// Removes the registry entry for `device_id`, returning a new config.
pub fn remove_device(config: &Value, device_id: &str) -> Value {
    let mut cfg = config.clone();
    if let Some(arr) = cfg
        .get_mut("agent")
        .and_then(|a| a.get_mut("devices"))
        .and_then(|d| d.as_array_mut())
    {
        arr.retain(|d| d.get("device_id").and_then(|v| v.as_str()) != Some(device_id));
    }
    cfg
}

// Config without the secret-bearing `agent` block (for broadcast to devices).
pub fn strip_agent(config: &Value) -> Value {
    let mut cfg = config.clone();
    if let Some(obj) = cfg.as_object_mut() {
        obj.remove("agent");
    }
    cfg
}

// (device_id, name) for every paired device, for the desktop panel.
pub fn paired_devices(config: &Value) -> Vec<(String, String)> {
    config
        .get("agent")
        .and_then(|a| a.get("devices"))
        .and_then(|d| d.as_array())
        .map(|arr| {
            arr.iter()
                .filter_map(|d| {
                    let id = d.get("device_id")?.as_str()?.to_string();
                    let name = d.get("name").and_then(|v| v.as_str()).unwrap_or("").to_string();
                    Some((id, name))
                })
                .collect()
        })
        .unwrap_or_default()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn mint_token_is_32_hex_chars() {
        let t = mint_token();
        assert_eq!(t.len(), 32);
        assert!(t.chars().all(|c| c.is_ascii_hexdigit()));
        assert_ne!(t, mint_token()); // overwhelmingly unlikely to collide
    }

    #[test]
    fn upsert_then_find_roundtrips() {
        let cfg = json!({"version": 1});
        let cfg = upsert_device(&cfg, "esp32-aa", "Desk", "tok123", 42);
        assert_eq!(find_paired_token(&cfg, "esp32-aa"), Some("tok123".to_string()));
        assert_eq!(find_paired_token(&cfg, "esp32-zz"), None);
    }

    #[test]
    fn upsert_replaces_same_device_id() {
        let cfg = json!({});
        let cfg = upsert_device(&cfg, "esp32-aa", "Old", "tok1", 1);
        let cfg = upsert_device(&cfg, "esp32-aa", "New", "tok2", 2);
        let arr = cfg["agent"]["devices"].as_array().unwrap();
        assert_eq!(arr.len(), 1);
        assert_eq!(find_paired_token(&cfg, "esp32-aa"), Some("tok2".to_string()));
    }

    #[test]
    fn remove_device_drops_entry() {
        let cfg = upsert_device(&json!({}), "esp32-aa", "Desk", "tok", 1);
        let cfg = remove_device(&cfg, "esp32-aa");
        assert_eq!(find_paired_token(&cfg, "esp32-aa"), None);
    }

    #[test]
    fn strip_agent_removes_only_agent() {
        let cfg = json!({"version": 1, "ha": {"token": "x"}, "agent": {"devices": []}});
        let stripped = strip_agent(&cfg);
        assert!(stripped.get("agent").is_none());
        assert!(stripped.get("ha").is_some());
        assert_eq!(stripped["version"], 1);
    }

    #[test]
    fn paired_devices_lists_id_and_name() {
        let cfg = upsert_device(&json!({}), "esp32-aa", "Desk", "tok", 1);
        assert_eq!(paired_devices(&cfg), vec![("esp32-aa".to_string(), "Desk".to_string())]);
    }
}
