# Stream Deck Per-Device Pairing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the single shared `agent.token` + hidden text field with per-device tokens, an explicit code-confirm pairing handshake, and dedicated pairing UI on both the desktop app and the device touchscreen.

**Architecture:** Desktop keeps a per-device registry (`agent.devices[]`) and mints a token per device on confirm; the `agent` block is stripped from every config broadcast so no device sees another's secret. Each device stores only its own token in `/deck/pairing.json`. Pairing is a code-confirm handshake: the device generates a 6-digit code it shows on-screen, the desktop shows the same code in a modal, the operator confirms the visual match.

**Tech Stack:** Rust (Tauri v2, axum WS, serde_json, getrandom), React/TypeScript, C++/Arduino/LVGL v9 (PlatformIO).

## Global Constraints

- Spec: `docs/superpowers/specs/2026-06-21-streamdeck-pairing-design.md` (authoritative).
- Wire contract lives in `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md` §3.1 — update it, don't fork it.
- Token = 16 random bytes hex-encoded → 32 lowercase hex chars. Code = 6 zero-padded decimal digits.
- Secrets (`agent` block) are NEVER broadcast to devices; `ha` block still is.
- Device's own token lives in `/deck/pairing.json` (NOT `config.json`).
- Firmware: PlatformIO CLI is at `~/.platformio/penv/bin/pio` (not on PATH). Single env `waveshare_esp32s3_35`. No firmware/JS test harness exists — those tasks are gated on a clean build + the stated manual check. Rust pure logic uses `cargo test`.
- Desktop dirs: Rust at `desktop/src-tauri/`, React at `desktop/src/`. Build checks: `cargo build` (run in `desktop/src-tauri/`), `npm run build` (run in `desktop/`).
- LVGL screens are built with `ui/styles.h` colors and returned as a fresh `lv_obj_create(NULL)` for `screen_manager::push()`.

---

### Task 1: Lock the protocol contract

**Files:**
- Modify: `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md` (§3.1)

Both the desktop and firmware tasks implement against this; update it first so they agree.

- [ ] **Step 1: Replace the §3.1 handshake block**

In `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md`, find the `### 3.1 Handshake / auth` section and replace its fenced code block + following paragraph with:

````markdown
### 3.1 Handshake / auth & pairing
```jsonc
// device → app, first message after connect (token from /deck/pairing.json, "" if unpaired)
{ "t": "hello", "device_id": "esp32-aabbcc", "fw": "1.0.0", "token": "" }

// app → device — token matched a registry entry
{ "t": "auth_ok" }

// app → device — no registry match: device must pair
{ "t": "pair_required" }

// device → app — device shows a 6-digit code on its screen, sends it up
{ "t": "pair_request", "device_id": "esp32-aabbcc", "code": "482917" }

// app → device — operator confirmed; app mints + stores a per-device token
{ "t": "paired", "token": "<32-char hex secret>" }

// app → device — operator rejected; app then closes the socket
{ "t": "pair_rejected" }
```
Per-device pairing: the desktop holds a registry `agent.devices[]` of
`{ device_id, name, token, paired_at }`. On `hello`, the app authenticates only
if `device_id` is in the registry **and** the presented token matches; any other
case yields `pair_required`. The device then generates a 6-digit code, displays
it, and sends `pair_request`; the operator confirms the on-screen code matches
the desktop modal before the app mints a token and replies `paired`. The
`agent` block is never broadcast to devices (§3.2). `run_command`/`launch_app`
`exec` requests are rejected on an unauthenticated connection.
````

- [ ] **Step 2: Commit**

```bash
git add docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md
git commit -m "docs: add per-device pairing handshake to protocol §3.1

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 2: Rust pairing registry helpers (pure, unit-tested)

**Files:**
- Create: `desktop/src-tauri/src/pairing.rs`
- Modify: `desktop/src-tauri/Cargo.toml` (add `getrandom`)
- Modify: `desktop/src-tauri/src/lib.rs:1-4` (add `mod pairing;`)

**Interfaces:**
- Produces:
  - `pairing::mint_token() -> String` (32 lowercase hex chars)
  - `pairing::find_paired_token(config: &serde_json::Value, device_id: &str) -> Option<String>`
  - `pairing::upsert_device(config: &Value, device_id: &str, name: &str, token: &str, paired_at: u64) -> Value`
  - `pairing::remove_device(config: &Value, device_id: &str) -> Value`
  - `pairing::strip_agent(config: &Value) -> Value`
  - `pairing::paired_devices(config: &Value) -> Vec<(String, String)>` (device_id, name)

- [ ] **Step 1: Add the getrandom dependency**

In `desktop/src-tauri/Cargo.toml`, under `[dependencies]`, after the `enigo = "0.2"` line add:

```toml
getrandom = "0.2"
```

- [ ] **Step 2: Write `pairing.rs` with the helpers and failing tests**

Create `desktop/src-tauri/src/pairing.rs`:

```rust
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
```

- [ ] **Step 3: Register the module**

In `desktop/src-tauri/src/lib.rs`, change the top module list (lines 1-4) from:

```rust
mod host_actions;
mod icons;
mod mdns;
mod ws_server;
```

to:

```rust
mod host_actions;
mod icons;
mod mdns;
mod pairing;
mod ws_server;
```

- [ ] **Step 4: Run the tests**

Run: `cd desktop/src-tauri && cargo test pairing`
Expected: PASS — 6 tests in `pairing::tests` pass.

- [ ] **Step 5: Commit**

```bash
git add desktop/src-tauri/Cargo.toml desktop/src-tauri/Cargo.lock desktop/src-tauri/src/pairing.rs desktop/src-tauri/src/lib.rs
git commit -m "feat(desktop): per-device pairing registry helpers

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 3: ws_server pairing handshake

**Files:**
- Modify: `desktop/src-tauri/src/ws_server.rs`

**Interfaces:**
- Consumes: `pairing::{find_paired_token, strip_agent, paired_devices}` (Task 2).
- Produces (used by Task 4):
  - `pub enum PairDecision { Approve(String), Reject }`
  - `pub async fn resolve_pair(state: &SharedState, device_id: &str, decision: PairDecision)`
  - `pub async fn disconnect_device(state: &SharedState, device_id: &str)`
  - `DeviceInfo { device_id: String, name: String, fw: String, online: bool }`
  - `pub async fn list_devices(...)` returns the registry merged with live state.

- [ ] **Step 1: Add imports and the PairDecision type**

In `desktop/src-tauri/src/ws_server.rs`, change the `use tokio::sync::{mpsc, Mutex};` line to:

```rust
use tokio::sync::{mpsc, oneshot, Mutex};
```

After the `pub type SharedState = Arc<ServerState>;` line, add:

```rust
// Outcome of a pairing prompt, delivered from a confirm/reject command to the
// socket task that is blocked waiting inside run_pairing().
pub enum PairDecision {
    Approve(String), // the minted token
    Reject,
}
```

- [ ] **Step 2: Replace DeviceInfo with the registry-merge shape**

Replace the `#[derive(Clone, Serialize)] pub struct DeviceInfo { ... }` block with:

```rust
#[derive(Clone, Serialize)]
pub struct DeviceInfo {
    pub device_id: String,
    pub name: String,
    pub fw: String,
    pub online: bool,
}
```

- [ ] **Step 3: Add the pending-pairs map to ServerState**

In `pub struct ServerState`, add a field after `clients: Mutex<HashMap<u64, Client>>,`:

```rust
    pending_pairs: Mutex<HashMap<String, oneshot::Sender<PairDecision>>>,
```

In `impl ServerState { pub fn new(...) }`, add to the constructed struct (after `clients: Mutex::new(HashMap::new()),`):

```rust
            pending_pairs: Mutex::new(HashMap::new()),
```

- [ ] **Step 4: Add the PairRequest variant to InMsg**

In the `enum InMsg`, after the `#[serde(rename = "exec")] Exec { id: u64, step: Value },` variant add:

```rust
    #[serde(rename = "pair_request")]
    PairRequest {
        #[allow(dead_code)]
        device_id: String,
        code: String,
    },
```

- [ ] **Step 5: Branch the hello handler into the pairing flow**

In `handle_socket`, replace the `Ok(InMsg::Hello { device_id, fw, token }) => { ... }` arm (the whole block that today computes `expected_token` and rejects) with:

```rust
            Ok(InMsg::Hello { device_id, fw, token }) => {
                let paired = {
                    let cfg = state.config.lock().await;
                    cfg.as_ref()
                        .and_then(|c| crate::pairing::find_paired_token(c, &device_id))
                };
                if paired.as_deref() == Some(token.as_str()) {
                    (device_id, fw) // already paired: token matches the registry
                } else {
                    // Unknown device or stale/empty token: run the pairing prompt.
                    match run_pairing(&state, &tx, &mut receiver, &device_id).await {
                        true => (device_id, fw),
                        false => {
                            drop(tx);
                            let _ = pump.await;
                            return;
                        }
                    }
                }
            }
```

- [ ] **Step 6: Add the run_pairing helper and delete the old expected_token fn**

Delete the entire `async fn expected_token(state: &SharedState) -> String { ... }` function. Then add, right after the `fn auth_fail(reason: &str) -> Message { ... }` function:

```rust
// Runs the code-confirm pairing handshake on an unauthenticated socket.
// Returns true once the operator approved (token already sent via `paired`),
// false if rejected or the device disconnected.
async fn run_pairing(
    state: &SharedState,
    tx: &mpsc::UnboundedSender<Message>,
    receiver: &mut futures_util::stream::SplitStream<WebSocket>,
    device_id: &str,
) -> bool {
    let _ = tx.send(Message::Text(json!({"t": "pair_required"}).to_string()));

    // Wait for the device to send its on-screen code.
    let code = loop {
        match receiver.next().await {
            Some(Ok(Message::Text(text))) => {
                if let Ok(InMsg::PairRequest { code, .. }) = serde_json::from_str::<InMsg>(&text) {
                    break code;
                }
            }
            Some(Ok(_)) => {} // ignore non-text frames while unpaired
            _ => return false, // socket closed / error
        }
    };

    let (otx, mut orx) = oneshot::channel::<PairDecision>();
    state.pending_pairs.lock().await.insert(device_id.to_string(), otx);
    let _ = state
        .app
        .emit("deck-pair-request", json!({"device_id": device_id, "code": code}));

    // Wait for the operator's decision, or the device disconnecting.
    let decision = loop {
        tokio::select! {
            d = &mut orx => break d.ok(),
            msg = receiver.next() => match msg {
                Some(Ok(_)) => continue, // stray frame: keep waiting
                _ => break None,         // disconnect
            }
        }
    };
    state.pending_pairs.lock().await.remove(device_id);

    match decision {
        Some(PairDecision::Approve(token)) => {
            let _ = tx.send(Message::Text(json!({"t": "paired", "token": token}).to_string()));
            true
        }
        _ => {
            let _ = state
                .app
                .emit("deck-pair-cancel", json!({"device_id": device_id}));
            let _ = tx.send(Message::Text(json!({"t": "pair_rejected"}).to_string()));
            let _ = tx.send(Message::Close(None));
            false
        }
    }
}

// Resolves a pending pairing prompt (called by the confirm/reject commands).
pub async fn resolve_pair(state: &SharedState, device_id: &str, decision: PairDecision) {
    if let Some(tx) = state.pending_pairs.lock().await.remove(device_id) {
        let _ = tx.send(decision);
    }
}

// Closes every live socket for `device_id` (called when forgetting a device).
pub async fn disconnect_device(state: &SharedState, device_id: &str) {
    let to_close: Vec<u64> = {
        let clients = state.clients.lock().await;
        clients
            .iter()
            .filter(|(_, c)| c.device_id == device_id)
            .map(|(id, _)| *id)
            .collect()
    };
    {
        let mut clients = state.clients.lock().await;
        for id in &to_close {
            if let Some(c) = clients.remove(id) {
                let _ = c.tx.send(Message::Close(None));
            }
        }
    }
    emit_devices(state).await;
}
```

- [ ] **Step 7: Strip the agent block from both push paths**

In `push_config_to`, change:

```rust
    let cfg = state.config.lock().await.clone();
    if let Some(cfg) = cfg {
        let rev = state.rev.load(Ordering::SeqCst);
```

to:

```rust
    let cfg = state.config.lock().await.clone();
    if let Some(cfg) = cfg {
        let cfg = crate::pairing::strip_agent(&cfg);
        let rev = state.rev.load(Ordering::SeqCst);
```

In `broadcast_config`, change:

```rust
pub async fn broadcast_config(state: &SharedState, config: Value) {
    *state.config.lock().await = Some(config.clone());
    let rev = state.rev.fetch_add(1, Ordering::SeqCst) + 1;
    let msg = json!({"t": "config", "config": config, "rev": rev}).to_string();
    let clients = state.clients.lock().await;
    for c in clients.values() {
        let _ = c.tx.send(Message::Text(msg.clone()));
        push_missing_icons(&state.app, &c.tx, &c.known_icons, &config);
    }
}
```

to:

```rust
pub async fn broadcast_config(state: &SharedState, config: Value) {
    *state.config.lock().await = Some(config.clone());
    let wire = crate::pairing::strip_agent(&config);
    let rev = state.rev.fetch_add(1, Ordering::SeqCst) + 1;
    let msg = json!({"t": "config", "config": wire, "rev": rev}).to_string();
    let clients = state.clients.lock().await;
    for c in clients.values() {
        let _ = c.tx.send(Message::Text(msg.clone()));
        push_missing_icons(&state.app, &c.tx, &c.known_icons, &wire);
    }
}
```

(Note: `push_missing_icons` reads `icons_referenced`, which scans `pages[]` — unaffected by stripping `agent`.)

- [ ] **Step 8: Rewrite list_devices and emit_devices to merge the registry**

Replace the whole `async fn emit_devices(state: &SharedState) { ... }` function with:

```rust
async fn build_device_list(state: &SharedState) -> Vec<DeviceInfo> {
    let registry = {
        let cfg = state.config.lock().await;
        cfg.as_ref().map(|c| crate::pairing::paired_devices(c)).unwrap_or_default()
    };
    let online: HashMap<String, String> = {
        let clients = state.clients.lock().await;
        clients.values().map(|c| (c.device_id.clone(), c.fw.clone())).collect()
    };
    registry
        .into_iter()
        .map(|(device_id, name)| {
            let fw = online.get(&device_id).cloned();
            DeviceInfo {
                online: fw.is_some(),
                fw: fw.unwrap_or_default(),
                name,
                device_id,
            }
        })
        .collect()
}

async fn emit_devices(state: &SharedState) {
    let devices = build_device_list(state).await;
    let _ = state.app.emit("deck-devices", devices);
}
```

Replace the whole `#[tauri::command] pub async fn list_devices(...) { ... }` function with:

```rust
#[tauri::command]
pub async fn list_devices(state: tauri::State<'_, SharedState>) -> Result<Vec<DeviceInfo>, String> {
    Ok(build_device_list(&state).await)
}
```

- [ ] **Step 9: Build**

Run: `cd desktop/src-tauri && cargo build`
Expected: compiles clean (warnings about the not-yet-wired `resolve_pair`/`disconnect_device`/`PairDecision` are acceptable until Task 4 wires them).

- [ ] **Step 10: Commit**

```bash
git add desktop/src-tauri/src/ws_server.rs
git commit -m "feat(desktop): WS pairing handshake + registry-merged device list

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 4: Desktop pairing commands

**Files:**
- Modify: `desktop/src-tauri/src/lib.rs`

**Interfaces:**
- Consumes: `ws_server::{resolve_pair, disconnect_device, PairDecision, broadcast_config}`, `pairing::{mint_token, upsert_device, remove_device}`.
- Produces: Tauri commands `confirm_pairing`, `reject_pairing`, `forget_device`.

- [ ] **Step 1: Add imports and a persist helper**

In `desktop/src-tauri/src/lib.rs`, change the `use` block near the top from:

```rust
use std::fs;
use std::path::PathBuf;
use tauri::Manager;
use ws_server::SharedState;
```

to:

```rust
use serde_json::{json, Value};
use std::fs;
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};
use tauri::Manager;
use ws_server::{PairDecision, SharedState};
```

After the `fn read_config_value(...)` function add:

```rust
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
```

- [ ] **Step 2: Add the three pairing commands**

After the `config_path_string` command in `desktop/src-tauri/src/lib.rs`, add:

```rust
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
```

- [ ] **Step 3: Register the commands**

In the `invoke_handler(tauri::generate_handler![ ... ])` list, change:

```rust
            config_path_string,
            ws_server::list_devices,
```

to:

```rust
            config_path_string,
            confirm_pairing,
            reject_pairing,
            forget_device,
            ws_server::list_devices,
```

- [ ] **Step 4: Build**

Run: `cd desktop/src-tauri && cargo build`
Expected: compiles clean, no warnings about unused `resolve_pair`/`disconnect_device`/`PairDecision` now.

- [ ] **Step 5: Commit**

```bash
git add desktop/src-tauri/src/lib.rs
git commit -m "feat(desktop): confirm/reject/forget pairing commands

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 5: Desktop types & API wrappers

**Files:**
- Modify: `desktop/src/types.ts`
- Modify: `desktop/src/api.ts`

**Interfaces:**
- Produces: `PairedDevice` type, `DeckConfig.agent` shape, `DeviceInfo` shape, and `confirmPairing`/`rejectPairing`/`forgetDevice` wrappers.

- [ ] **Step 1: Update the config types**

In `desktop/src/types.ts`, replace:

```ts
export interface DeckConfig {
  version: number;
  grid: { cols: number; rows: number };
  agent?: { token: string };
  ha?: { base_url: string; token: string };
  pages: DeckPage[];
}
```

with:

```ts
export interface PairedDevice {
  device_id: string;
  name: string;
  token: string;
  paired_at: number;
}

export interface DeckConfig {
  version: number;
  grid: { cols: number; rows: number };
  agent?: { devices: PairedDevice[] };
  ha?: { base_url: string; token: string };
  pages: DeckPage[];
}
```

In the same file, in `emptyConfig()`, change `agent: { token: "" },` to `agent: { devices: [] },`.

- [ ] **Step 2: Update DeviceInfo and add the command wrappers**

In `desktop/src/api.ts`, replace:

```ts
export interface DeviceInfo {
  conn_id: number;
  device_id: string;
  fw: string;
  authed: boolean;
}
```

with:

```ts
export interface DeviceInfo {
  device_id: string;
  name: string;
  fw: string;
  online: boolean;
}

export async function confirmPairing(deviceId: string, name: string): Promise<void> {
  await invoke("confirm_pairing", { deviceId, name });
}

export async function rejectPairing(deviceId: string): Promise<void> {
  await invoke("reject_pairing", { deviceId });
}

export async function forgetDevice(deviceId: string): Promise<void> {
  await invoke("forget_device", { deviceId });
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: `tsc` reports errors in `TopBar.tsx` (still references `config.agent.token`) and `DevicesPanel.tsx` (references `conn_id`/`authed`). That is expected — Tasks 6-7 fix them. Confirm the errors are ONLY in those two files; `types.ts`/`api.ts` themselves must compile.

- [ ] **Step 4: Commit**

```bash
git add desktop/src/types.ts desktop/src/api.ts
git commit -m "feat(desktop): per-device pairing types + command wrappers

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 6: PairModal component + App wiring

**Files:**
- Create: `desktop/src/components/PairModal.tsx`
- Modify: `desktop/src/App.tsx`
- Modify: `desktop/src/App.css` (append modal styles)

**Interfaces:**
- Consumes: `confirmPairing`, `rejectPairing` (Task 5); Tauri events `deck-pair-request` / `deck-pair-cancel` (Task 3).

- [ ] **Step 1: Create the modal component**

Create `desktop/src/components/PairModal.tsx`:

```tsx
import { useState } from "react";
import { confirmPairing, rejectPairing } from "../api";

export interface PendingPair {
  device_id: string;
  code: string;
}

interface Props {
  pending: PendingPair;
  onDone: () => void;
}

export default function PairModal({ pending, onDone }: Props) {
  const [name, setName] = useState("");
  const [busy, setBusy] = useState(false);

  async function pair() {
    setBusy(true);
    try {
      await confirmPairing(pending.device_id, name.trim() || pending.device_id);
      onDone();
    } finally {
      setBusy(false);
    }
  }

  async function reject() {
    setBusy(true);
    try {
      await rejectPairing(pending.device_id);
      onDone();
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="modal-overlay">
      <div className="modal pair-modal">
        <h2>New device wants to pair</h2>
        <p className="pair-device-id">{pending.device_id}</p>
        <p className="pair-instructions">
          Confirm this code matches the one shown on the device screen:
        </p>
        <div className="pair-code">{pending.code.split("").join(" ")}</div>
        <label className="pair-name">
          Name (optional)
          <input
            value={name}
            placeholder={pending.device_id}
            onChange={(e) => setName(e.target.value)}
          />
        </label>
        <div className="pair-actions">
          <button type="button" className="reject" onClick={reject} disabled={busy}>
            Reject
          </button>
          <button type="button" className="confirm" onClick={pair} disabled={busy}>
            Pair
          </button>
        </div>
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Wire the events + modal into App**

In `desktop/src/App.tsx`, add to the imports at the top:

```tsx
import { listen } from "@tauri-apps/api/event";
import PairModal, { PendingPair } from "./components/PairModal";
```

Inside the `App` component, after the existing `const [path, setPath] = useState("");` line add:

```tsx
  const [pending, setPending] = useState<PendingPair | null>(null);
```

After the existing first `useEffect`, add a second one:

```tsx
  useEffect(() => {
    const unReq = listen<PendingPair>("deck-pair-request", (e) => setPending(e.payload));
    const unCancel = listen<{ device_id: string }>("deck-pair-cancel", (e) =>
      setPending((p) => (p && p.device_id === e.payload.device_id ? null : p)),
    );
    return () => {
      unReq.then((f) => f());
      unCancel.then((f) => f());
    };
  }, []);
```

In the returned JSX, immediately after the opening `<div className="app">` add:

```tsx
      {pending && <PairModal pending={pending} onDone={() => setPending(null)} />}
```

- [ ] **Step 3: Append modal styles**

Append to `desktop/src/App.css`:

```css
.modal-overlay {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.6);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 100;
}
.modal {
  background: #1e1e24;
  border: 1px solid #34343c;
  border-radius: 12px;
  padding: 24px;
  min-width: 320px;
  color: #e6e6ea;
}
.pair-modal h2 {
  margin: 0 0 8px;
  font-size: 1.1rem;
}
.pair-device-id {
  font-family: monospace;
  color: #9a9aa6;
  margin: 0 0 12px;
}
.pair-instructions {
  margin: 0 0 12px;
  color: #c4c4ce;
}
.pair-code {
  font-size: 2rem;
  font-weight: 700;
  letter-spacing: 0.2em;
  text-align: center;
  padding: 12px;
  background: #14141a;
  border-radius: 8px;
  margin-bottom: 16px;
}
.pair-name {
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin-bottom: 16px;
}
.pair-actions {
  display: flex;
  justify-content: flex-end;
  gap: 8px;
}
.pair-actions .confirm {
  background: #3ecf8e;
  color: #08130d;
  font-weight: 600;
}
.pair-actions .reject {
  background: transparent;
  color: #d66;
  border: 1px solid #d66;
}
```

- [ ] **Step 4: Build**

Run: `cd desktop && npm run build`
Expected: errors now only in `DevicesPanel.tsx` and `TopBar.tsx` (fixed in Task 7); `PairModal.tsx` and `App.tsx` compile.

- [ ] **Step 5: Commit**

```bash
git add desktop/src/components/PairModal.tsx desktop/src/App.tsx desktop/src/App.css
git commit -m "feat(desktop): pairing confirm modal wired to pair events

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 7: Paired Devices panel + remove token field

**Files:**
- Modify: `desktop/src/components/DevicesPanel.tsx`
- Modify: `desktop/src/components/TopBar.tsx`
- Modify: `desktop/src/App.css` (append panel styles)

**Interfaces:**
- Consumes: `listDevices`, `forgetDevice`, `DeviceInfo` (Task 5).

- [ ] **Step 1: Rework DevicesPanel into a registry + forget list**

Replace the entire contents of `desktop/src/components/DevicesPanel.tsx` with:

```tsx
import { useEffect, useState } from "react";
import { DeviceInfo, forgetDevice, listDevices } from "../api";

export default function DevicesPanel() {
  const [devices, setDevices] = useState<DeviceInfo[]>([]);

  useEffect(() => {
    let cancelled = false;
    const poll = () => {
      listDevices().then((list) => {
        if (!cancelled) setDevices(list);
      });
    };
    poll();
    const id = setInterval(poll, 1500);
    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, []);

  async function forget(device_id: string) {
    await forgetDevice(device_id);
    setDevices((ds) => ds.filter((d) => d.device_id !== device_id));
  }

  return (
    <div className="devices-panel">
      <span className="devices-label">Paired Devices</span>
      {devices.length === 0 ? (
        <span className="devices-empty">none paired</span>
      ) : (
        devices.map((d) => (
          <span className="device-chip" key={d.device_id}>
            <span className={`device-dot ${d.online ? "online" : "offline"}`} />
            {d.name || d.device_id}
            <span className="device-meta">
              {d.device_id}
              {d.online ? ` · fw ${d.fw}` : " · offline"}
            </span>
            <button type="button" className="forget-btn" onClick={() => forget(d.device_id)}>
              Forget
            </button>
          </span>
        ))
      )}
    </div>
  );
}
```

- [ ] **Step 2: Remove the token field from TopBar**

In `desktop/src/components/TopBar.tsx`, delete the `randomToken()` helper function at the top, and delete the entire `<label> Agent pairing token … </label>` block (the `<label>` containing the `token-row` span). Leave the Grid and HA fields untouched.

- [ ] **Step 3: Append panel styles**

Append to `desktop/src/App.css`:

```css
.device-dot.online {
  background: #3ecf8e;
}
.device-dot.offline {
  background: transparent;
  border: 1px solid #6a6a72;
}
.device-meta {
  color: #8a8a94;
  font-size: 0.8rem;
  margin-left: 6px;
}
.forget-btn {
  margin-left: 8px;
  font-size: 0.75rem;
  background: transparent;
  color: #d66;
  border: 1px solid #d66;
  border-radius: 4px;
  padding: 1px 6px;
}
```

- [ ] **Step 4: Build**

Run: `cd desktop && npm run build`
Expected: PASS — `tsc` + `vite build` complete with no errors.

- [ ] **Step 5: Commit**

```bash
git add desktop/src/components/DevicesPanel.tsx desktop/src/components/TopBar.tsx desktop/src/App.css
git commit -m "feat(desktop): paired devices panel with forget; drop token field

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 8: Firmware pairing.json storage

**Files:**
- Modify: `src/core/deck_client.cpp`

**Interfaces:**
- Produces (used within deck_client): `load_agent_token()` reads `/deck/pairing.json`; `save_pairing(device_id, token)` writes it.

- [ ] **Step 1: Replace load_agent_token to read pairing.json + add save_pairing**

In `src/core/deck_client.cpp`, replace the `static String load_agent_token()` function:

```cpp
static String load_agent_token()
{
  deck_config::config c;
  if (!deck_config::load(c)) return "";
  return c.agent_token;
}
```

with:

```cpp
static constexpr const char *PAIRING_PATH = "/deck/pairing.json";

// The device's own pairing token, persisted separately from the synced config
// (which is overwritten on every push). Empty string => unpaired.
static String load_agent_token()
{
  if (!sdcard::is_mounted() || !sdcard::exists(PAIRING_PATH)) return "";
  String json = sdcard::read_string(PAIRING_PATH);
  JsonDocument doc;
  if (deserializeJson(doc, json)) return "";
  return (const char *)(doc["token"] | "");
}

static void save_pairing(const String &device_id, const String &token)
{
  sdcard::mkdir("/deck"); // no-op if it already exists
  JsonDocument doc;
  doc["device_id"] = device_id;
  doc["token"] = token;
  String out;
  serializeJson(doc, out);
  if (sdcard::write(PAIRING_PATH, out.c_str())) {
    LOG_I("deck_client", "pairing token saved");
  } else {
    LOG_E("deck_client", "failed to write %s", PAIRING_PATH);
  }
}
```

- [ ] **Step 2: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles (a warning that `save_pairing` is unused until Task 9 is acceptable).

- [ ] **Step 3: Commit**

```bash
git add src/core/deck_client.cpp
git commit -m "feat(fw): store device pairing token in /deck/pairing.json

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 9: Firmware pairing state machine in deck_client

**Files:**
- Modify: `src/core/deck_client.h`
- Modify: `src/core/deck_client.cpp`

**Interfaces:**
- Produces (used by Tasks 10-11):
  - `enum deck_client::pairing_state_t { PAIR_IDLE, PAIR_PENDING, PAIR_SUCCESS, PAIR_REJECTED }`
  - `struct deck_client::pairing_status { pairing_state_t state; char code[7]; }`
  - `deck_client::pairing_status deck_client::pairing_state()`
  - `void deck_client::clear_pairing()`

- [ ] **Step 1: Declare the pairing API in the header**

In `src/core/deck_client.h`, after the `bool connected();` declaration (and its doc comment block) add:

```cpp
// Pairing handshake state, polled from the LVGL thread to drive the on-device
// pairing screen (see apps/deck/deck_pairing). `code` is the 6-digit code shown
// on the device and sent to the desktop for visual confirmation.
enum pairing_state_t { PAIR_IDLE = 0, PAIR_PENDING, PAIR_SUCCESS, PAIR_REJECTED };

struct pairing_status {
  pairing_state_t state;
  char code[7];
};

pairing_status pairing_state();

// Resets pairing state to PAIR_IDLE. Called by the pairing screen once it has
// shown the success/rejected result and is about to pop itself.
void clear_pairing();
```

- [ ] **Step 2: Add pairing state + handlers in the implementation**

In `src/core/deck_client.cpp`, add the include for the ESP RNG near the other includes:

```cpp
#include <esp_system.h>
```

After the `static char s_pending_err[64] = {0};` line (the exec/result state block), add:

```cpp
// Pairing handshake state (see pairing_state()/handle_pair_*).
static volatile pairing_state_t s_pair_state = PAIR_IDLE;
static char s_pair_code[7] = {0};

static void gen_pair_code()
{
  uint32_t r = esp_random() % 1000000u;
  snprintf(s_pair_code, sizeof(s_pair_code), "%06u", (unsigned)r);
}

static void send_pair_request()
{
  JsonDocument doc;
  doc["t"] = "pair_request";
  doc["device_id"] = s_device_id;
  doc["code"] = s_pair_code;
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}

static void handle_pair_required()
{
  gen_pair_code();
  s_pair_state = PAIR_PENDING;
  LOG_I("deck_client", "pairing required, code %s", s_pair_code);
  send_pair_request();
}

static void handle_paired(JsonDocument &doc)
{
  String token = (const char *)(doc["token"] | "");
  if (token.isEmpty()) return;
  save_pairing(s_device_id, token);
  s_authed = true;
  s_pair_state = PAIR_SUCCESS;
  LOG_I("deck_client", "paired");
}

static void handle_pair_rejected()
{
  s_pair_state = PAIR_REJECTED;
  LOG_W("deck_client", "pairing rejected by desktop");
}
```

- [ ] **Step 3: Dispatch the new message types**

In `handle_message`, extend the `if/else` chain. After the `} else if (!strcmp(t, "result")) { handle_result(doc); }` branch and before the trailing comment, add:

```cpp
  } else if (!strcmp(t, "pair_required")) {
    handle_pair_required();
  } else if (!strcmp(t, "paired")) {
    handle_paired(doc);
  } else if (!strcmp(t, "pair_rejected")) {
    handle_pair_rejected();
```

- [ ] **Step 4: Reset pairing state on (dis)connect**

In `on_event`, in the `case WStype_CONNECTED:` block, after `s_authed = false;` add:

```cpp
      s_pair_state = PAIR_IDLE;
```

In `case WStype_DISCONNECTED:`, after the existing exec-unblock `if (...) { ... }` block, add:

```cpp
      if (s_pair_state == PAIR_PENDING) s_pair_state = PAIR_IDLE; // pop the pairing screen
```

- [ ] **Step 5: Implement the public pairing accessors**

At the end of `src/core/deck_client.cpp`, before the closing `} // namespace deck_client`, add:

```cpp
pairing_status pairing_state()
{
  pairing_status s;
  s.state = s_pair_state;
  strncpy(s.code, s_pair_code, sizeof(s.code) - 1);
  s.code[sizeof(s.code) - 1] = '\0';
  return s;
}

void clear_pairing()
{
  s_pair_state = PAIR_IDLE;
}
```

- [ ] **Step 6: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles clean.

- [ ] **Step 7: Commit**

```bash
git add src/core/deck_client.h src/core/deck_client.cpp
git commit -m "feat(fw): pairing handshake state machine in deck_client

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 10: Firmware pairing screen (LVGL)

**Files:**
- Create: `src/apps/deck/deck_pairing.h`
- Create: `src/apps/deck/deck_pairing.cpp`

**Interfaces:**
- Consumes: `deck_client::pairing_state()`, `deck_client::clear_pairing()` (Task 9); `screen_manager::pop()`; `ui/styles.h`.
- Produces: `lv_obj_t *deck_pairing_create()` (used by Task 11).

- [ ] **Step 1: Create the header**

Create `src/apps/deck/deck_pairing.h`:

```cpp
#pragma once

#include <lvgl.h>

// Full-screen pairing prompt: shows the device-generated 6-digit code and
// instructs the operator to confirm it on the desktop app. Polls
// deck_client::pairing_state(); on success/rejection it shows the result
// briefly, then pops itself (calling deck_client::clear_pairing() first).
// Pushed by main.cpp when pairing becomes pending; returns a fresh screen for
// screen_manager::push().
lv_obj_t *deck_pairing_create();
```

- [ ] **Step 2: Create the implementation**

Create `src/apps/deck/deck_pairing.cpp`:

```cpp
#include "apps/deck/deck_pairing.h"

#include "core/deck_client.h"
#include "core/screen_manager.h"
#include "ui/styles.h"

static lv_obj_t *s_proot = nullptr;
static lv_obj_t *s_pcode = nullptr;
static lv_obj_t *s_pstatus = nullptr;
static lv_timer_t *s_ptimer = nullptr;
static bool s_popping = false;

static void pop_now_cb(lv_timer_t *t)
{
  lv_timer_delete(t);
  deck_client::clear_pairing();
  screen_manager::pop();
}

static void schedule_pop(uint32_t delay_ms)
{
  if (s_popping) return;
  s_popping = true;
  // pop_now_cb deletes this timer itself on its first (only) firing.
  lv_timer_create(pop_now_cb, delay_ms, nullptr);
}

static void ptimer_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  deck_client::pairing_status ps = deck_client::pairing_state();
  switch (ps.state) {
    case deck_client::PAIR_PENDING:
      lv_label_set_text(s_pcode, ps.code);
      break;
    case deck_client::PAIR_SUCCESS:
      lv_label_set_text(s_pstatus, LV_SYMBOL_OK " Paired");
      lv_obj_set_style_text_color(s_pstatus, styles::accent_green(), 0);
      schedule_pop(900);
      break;
    case deck_client::PAIR_REJECTED:
      lv_label_set_text(s_pstatus, LV_SYMBOL_CLOSE " Rejected");
      lv_obj_set_style_text_color(s_pstatus, styles::accent_red(), 0);
      schedule_pop(1500);
      break;
    case deck_client::PAIR_IDLE:
    default:
      // Disconnected while pending — drop back to whatever was underneath.
      schedule_pop(0);
      break;
  }
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_ptimer) {
    lv_timer_delete(s_ptimer);
    s_ptimer = nullptr;
  }
  s_proot = nullptr;
  s_pcode = nullptr;
  s_pstatus = nullptr;
  s_popping = false;
}

lv_obj_t *deck_pairing_create()
{
  s_popping = false;
  s_proot = lv_obj_create(NULL);
  lv_obj_add_style(s_proot, &styles::style_screen, 0);
  lv_obj_remove_flag(s_proot, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(s_proot, screen_delete_cb, LV_EVENT_DELETE, NULL);

  lv_obj_t *title = lv_label_create(s_proot);
  lv_label_set_text(title, "Pairing");
  lv_obj_set_style_text_color(title, styles::text_secondary(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

  s_pcode = lv_label_create(s_proot);
  deck_client::pairing_status ps = deck_client::pairing_state();
  lv_label_set_text(s_pcode, ps.code[0] ? ps.code : "------");
  lv_obj_set_style_text_color(s_pcode, styles::text_primary(), 0);
  lv_obj_set_style_text_font(s_pcode, &lv_font_montserrat_28, 0);
  lv_obj_align(s_pcode, LV_ALIGN_CENTER, 0, -20);

  lv_obj_t *hint = lv_label_create(s_proot);
  lv_label_set_text(hint, "Confirm this code on your\ncomputer to pair this deck.");
  lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(hint, 280);
  lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(hint, styles::text_muted(), 0);
  lv_obj_align(hint, LV_ALIGN_CENTER, 0, 40);

  s_pstatus = lv_label_create(s_proot);
  lv_label_set_text(s_pstatus, "waiting...");
  lv_obj_set_style_text_color(s_pstatus, styles::text_muted(), 0);
  lv_obj_align(s_pstatus, LV_ALIGN_BOTTOM_MID, 0, -40);

  s_ptimer = lv_timer_create(ptimer_cb, 200, NULL);
  return s_proot;
}
```

- [ ] **Step 3: Verify the font symbol is available**

Run: `grep -n "LV_FONT_MONTSERRAT_28\|LV_FONT_MONTSERRAT_28 " include/lv_conf.h`
Expected: a line `#define LV_FONT_MONTSERRAT_28 1`. If it shows `0`, change it to `1` in `include/lv_conf.h` (the code uses `lv_font_montserrat_28` for the large code), and include that file change in this task's commit.

- [ ] **Step 4: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles clean (a warning that `deck_pairing_create` is unused until Task 11 is acceptable).

- [ ] **Step 5: Commit**

```bash
git add src/apps/deck/deck_pairing.h src/apps/deck/deck_pairing.cpp include/lv_conf.h
git commit -m "feat(fw): on-device pairing screen

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 11: Auto-switch to the pairing screen from main loop

**Files:**
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: `deck_client::pairing_state()` (Task 9), `deck_pairing_create()` (Task 10), `screen_manager::push()`.

- [ ] **Step 1: Include the pairing screen header**

In `src/main.cpp`, near the other deck-related includes (where `deck_client.h` / the deck app are included), add:

```cpp
#include "apps/deck/deck_pairing.h"
```

If `screen_manager.h` is not already included in `main.cpp`, add `#include "core/screen_manager.h"` as well (check the existing include block first).

- [ ] **Step 2: Push the pairing screen on the pending edge**

In `loop()`, replace:

```cpp
  if (deck_client::consume_config_pushed() || deck_client::consume_icons_pushed()) {
    deck_invalidate_config();
  }
```

with:

```cpp
  if (deck_client::consume_config_pushed() || deck_client::consume_icons_pushed()) {
    deck_invalidate_config();
  }

  // Surface the pairing screen the moment the device needs to pair, over
  // whatever app is open. The screen pops itself once pairing resolves (which
  // resets state to PAIR_IDLE via deck_client::clear_pairing()).
  static bool s_pair_open = false;
  deck_client::pairing_status ps = deck_client::pairing_state();
  if (ps.state == deck_client::PAIR_PENDING && !s_pair_open) {
    screen_manager::push(deck_pairing_create());
    s_pair_open = true;
  } else if (ps.state == deck_client::PAIR_IDLE && s_pair_open) {
    s_pair_open = false;
  }
```

- [ ] **Step 3: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles clean.

- [ ] **Step 4: Manual end-to-end verification**

Flash and run both ends:
- `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35 -t upload -t monitor`
- `cd desktop && npm run tauri dev`

Verify, in order:
1. **First pair:** delete `/deck/pairing.json` on the SD card (or use a fresh card). On boot the device auto-switches to the Pairing screen showing a 6-digit code; the desktop shows the PairModal with the same code. Click **Pair** → device shows "✓ Paired", returns to the launcher/deck, and the grid loads. The device now appears in **Paired Devices** with a green dot.
2. **Reconnect:** power-cycle the device → it reconnects with the stored token and goes straight to the deck (no pairing prompt).
3. **Reject:** delete `/deck/pairing.json`, reboot, click **Reject** on the modal → device shows "✗ Rejected" then returns; no entry added.
4. **Forget + re-pair:** click **Forget** on the paired device → it leaves the panel and the socket drops; the device reconnects and re-enters pairing.

- [ ] **Step 5: Commit**

```bash
git add src/main.cpp
git commit -m "feat(fw): auto-switch to pairing screen when pairing is needed

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Notes on testing strategy

This project has no firmware or JS test harness ("No tests/linters are configured" per CLAUDE.md), so only the pure Rust registry logic (Task 2) gets `cargo test` coverage. Every other task is gated on a clean build (`cargo build` / `npm run build` / `pio run`) plus the explicit manual check in Task 11 Step 4, which exercises the full first-pair / reconnect / reject / forget matrix from the spec's §6.
