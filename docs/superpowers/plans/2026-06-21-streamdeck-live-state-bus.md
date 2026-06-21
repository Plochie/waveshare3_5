# Stream Deck Live State Bus Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a bidirectional key/value "live state" bus — desktop is the hub, pushes values to devices so tiles reflect them (toggle/text), and a tile tap sets a value back, keeping all devices consistent.

**Architecture:** Desktop holds an in-memory `key→value` store in the existing axum WS server; `set_value()` updates it, broadcasts a `state` delta to all devices, and emits a Tauri event. Sources (a State tab + an HTTP `/kv` route) feed it. Devices cache values, get a `state_snapshot` on connect, render bound tiles, and send `set` on a toggle tap.

**Tech Stack:** Rust (Tauri v2, axum 0.7), React/TypeScript, C++/Arduino/LVGL v9 (PlatformIO).

## Global Constraints

- Spec: `docs/superpowers/specs/2026-06-21-streamdeck-live-state-bus-design.md` (authoritative).
- Wire contract lives in `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md` §3.5 — update it (Task 1), don't fork it.
- Values are **strings** on the wire. Messages: `state_snapshot {values:{k:v}}` and `state {key,value}` (app→device); `set {key,value}` (device→app). Unknown `t`/keys are ignored.
- `bind` defaults: `on_value` `"1"`, `off_value` `"0"`, `mode` `"toggle"`. A `toggle` tap sends a `set` (does not run `steps`); `bind` and `open_page` are mutually exclusive (`open_page` wins).
- The store is in-memory only (live state; not persisted across desktop restart). No per-key subscription. `set` is honored only on an authenticated connection.
- v1 sources: manual UI + HTTP `/kv` only. Host-state and Home Assistant are out of scope (later cycles).
- Build checks: Rust `cargo build`/`cargo test` in `desktop/src-tauri/`; frontend `npm run build` in `desktop/`; firmware `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`. No JS/firmware test harness — those tasks are build-gated; final firmware task has a manual E2E pass.

---

### Task 1: Lock the protocol contract (§3.5)

**Files:**
- Modify: `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md` (§3.5)

- [ ] **Step 1: Replace the §3.5 block**

In `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md`, replace the `### 3.5 Live state (Phase 4, reserved)` heading and its code block + following line with:

````markdown
### 3.5 Live state bus (key/value)
```jsonc
// app → device, right after auth_ok + config: current store snapshot
{ "t": "state_snapshot", "values": { "mic_muted": "1", "cpu_temp": "62" } }
// app → device, on each value change (broadcast to every connected device)
{ "t": "state", "key": "mic_muted", "value": "0" }
// device → app, when a toggle-bound tile is tapped
{ "t": "set", "key": "mic_muted", "value": "0" }
```
The desktop is the authoritative hub: it holds an in-memory `key→value` store
(values are strings). Any source (UI, the `/kv` HTTP endpoint, later host/HA
adapters) calls a single set path that stores the value and broadcasts a `state`
delta to all devices. A device caches values, receives a `state_snapshot` on
connect, and renders tiles bound to a key. A toggle tile's tap sends `set`; the
hub stores it and re-broadcasts `state` to everyone (including the sender).
`set` is honored only on an authenticated connection. Unknown `t` values and
unknown keys are ignored (forward-compatible).
````

- [ ] **Step 2: Commit**

```bash
git add docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md
git commit -m "docs: define live-state key/value bus in protocol §3.5

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 2: Desktop backend — store, set_value, snapshot, Set handler, HTTP, commands

**Files:**
- Modify: `desktop/src-tauri/src/ws_server.rs`
- Modify: `desktop/src-tauri/src/lib.rs`

**Interfaces:**
- Produces: `ServerState.state: Mutex<HashMap<String,String>>`; `pub async fn ws_server::set_value(state: &SharedState, key: String, value: String)`; `InMsg::Set`; HTTP routes `POST /kv` + `GET /kv/set`; Tauri commands `set_state`, `list_state`.

- [ ] **Step 1: Add the store field to ServerState**

In `desktop/src-tauri/src/ws_server.rs`, add a field to `pub struct ServerState` after `pending_pairs: ...,`:

```rust
    pub state: Mutex<HashMap<String, String>>,
```

And in `impl ServerState { pub fn new(...) }`, after `pending_pairs: Mutex::new(HashMap::new()),`:

```rust
            state: Mutex::new(HashMap::new()),
```

- [ ] **Step 2: Add imports for the HTTP routes**

In `ws_server.rs`, change the axum import block and routing import:

```rust
use axum::{
    extract::{
        ws::{Message, WebSocket, WebSocketUpgrade},
        Query, State,
    },
    response::IntoResponse,
    routing::{get, post},
    Json, Router,
};
```

- [ ] **Step 3: Add the InMsg::Set variant**

In `enum InMsg`, after the `PairRequest { ... }` variant add:

```rust
    #[serde(rename = "set")]
    Set { key: String, value: String },
```

- [ ] **Step 4: Add set_value + the snapshot builder + unit test**

In `ws_server.rs`, add near the other broadcast helpers (e.g. after `broadcast_config`):

```rust
// Builds the state_snapshot message JSON from the current store.
fn build_snapshot(map: &HashMap<String, String>) -> String {
    let values: serde_json::Map<String, Value> = map
        .iter()
        .map(|(k, v)| (k.clone(), Value::String(v.clone())))
        .collect();
    json!({ "t": "state_snapshot", "values": values }).to_string()
}

// The single set path: store the value, broadcast a `state` delta to every
// connected device, and notify the desktop UI. All sources funnel through here.
pub async fn set_value(state: &SharedState, key: String, value: String) {
    state.state.lock().await.insert(key.clone(), value.clone());
    let msg = json!({ "t": "state", "key": key, "value": value }).to_string();
    {
        let clients = state.clients.lock().await;
        for c in clients.values() {
            let _ = c.tx.send(Message::Text(msg.clone()));
        }
    }
    let _ = state.app.emit("deck-state", json!({ "key": key, "value": value }));
}

#[derive(Deserialize)]
struct KvParams {
    key: String,
    value: String,
}

async fn kv_post(State(state): State<SharedState>, Json(p): Json<KvParams>) -> impl IntoResponse {
    set_value(&state, p.key, p.value).await;
    "ok"
}

async fn kv_get(State(state): State<SharedState>, Query(p): Query<KvParams>) -> impl IntoResponse {
    set_value(&state, p.key, p.value).await;
    "ok"
}

#[cfg(test)]
mod state_tests {
    use super::*;

    #[test]
    fn snapshot_has_type_and_values() {
        let mut m = HashMap::new();
        m.insert("a".to_string(), "1".to_string());
        let s = build_snapshot(&m);
        let v: Value = serde_json::from_str(&s).unwrap();
        assert_eq!(v["t"], "state_snapshot");
        assert_eq!(v["values"]["a"], "1");
    }

    #[test]
    fn snapshot_empty_is_object() {
        let v: Value = serde_json::from_str(&build_snapshot(&HashMap::new())).unwrap();
        assert!(v["values"].is_object());
        assert_eq!(v["values"].as_object().unwrap().len(), 0);
    }
}
```

- [ ] **Step 5: Register the HTTP routes**

In `pub async fn run(...)`, change the router line:

```rust
    let router = Router::new().route("/ws", get(ws_handler)).with_state(state.clone());
```

to:

```rust
    let router = Router::new()
        .route("/ws", get(ws_handler))
        .route("/kv", post(kv_post))
        .route("/kv/set", get(kv_get))
        .with_state(state.clone());
```

- [ ] **Step 6: Send the snapshot on connect**

In `handle_socket`, right after the existing `push_config_to(&tx, &state).await;` line, add:

```rust
    {
        let snap = build_snapshot(&*state.state.lock().await);
        let _ = tx.send(Message::Text(snap));
    }
```

- [ ] **Step 7: Handle device `set` in the receive loop**

In `handle_socket`'s `while let Some(Ok(msg)) = receiver.next().await` match, add an arm (next to `InMsg::Exec`):

```rust
            Ok(InMsg::Set { key, value }) => {
                // §3.5: device -> app on a toggle tap. Already authenticated
                // here. Store + re-broadcast to all (including this sender).
                set_value(&state, key, value).await;
            }
```

- [ ] **Step 8: Add the Tauri commands in lib.rs**

In `desktop/src-tauri/src/lib.rs`, add (after the existing pairing commands):

```rust
#[tauri::command]
async fn set_state(
    state: tauri::State<'_, SharedState>,
    key: String,
    value: String,
) -> Result<(), String> {
    ws_server::set_value(&state, key, value).await;
    Ok(())
}

#[tauri::command]
async fn list_state(
    state: tauri::State<'_, SharedState>,
) -> Result<std::collections::HashMap<String, String>, String> {
    Ok(state.state.lock().await.clone())
}
```

And register them in the `tauri::generate_handler![ ... ]` list (after `forget_device,`):

```rust
            set_state,
            list_state,
```

- [ ] **Step 9: Build + test**

Run: `cd desktop/src-tauri && cargo test state_tests && cargo build`
Expected: 2 `state_tests` pass; build compiles clean.

- [ ] **Step 10: Commit**

```bash
git add desktop/src-tauri/src/ws_server.rs desktop/src-tauri/src/lib.rs
git commit -m "feat(desktop): live-state key/value store, set_value, snapshot, /kv, set handler

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 3: Desktop types + API wrappers

**Files:**
- Modify: `desktop/src/types.ts`
- Modify: `desktop/src/api.ts`

**Interfaces:**
- Produces: `Bind` interface + `bind?: Bind` on `DeckButton`; `setState(key, value)`, `listState()` wrappers.

- [ ] **Step 1: Add the Bind type to DeckButton**

In `desktop/src/types.ts`, add before `export interface DeckButton`:

```ts
export interface Bind {
  key: string;
  mode: "toggle" | "text";
  on_value?: string;
  off_value?: string;
  on_label?: string;
  off_label?: string;
  on_color?: string;
  off_color?: string;
  format?: string;
}
```

And add a field to `DeckButton`:

```ts
export interface DeckButton {
  pos: number;
  label: string;
  icon?: string;
  color?: string;
  open_page?: string;
  steps?: Step[];
  bind?: Bind;
}
```

- [ ] **Step 2: Add the API wrappers**

In `desktop/src/api.ts`, add (after the existing wrappers):

```ts
export async function setState(key: string, value: string): Promise<void> {
  await invoke("set_state", { key, value });
}

export async function listState(): Promise<Record<string, string>> {
  return invoke<Record<string, string>>("list_state");
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS (the new type/wrappers compile; unused until Tasks 4-5).

- [ ] **Step 4: Commit**

```bash
git add desktop/src/types.ts desktop/src/api.ts
git commit -m "feat(desktop): Bind type + live-state API wrappers

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 4: Desktop State tab (StatePage + AppTabs + App wiring)

**Files:**
- Create: `desktop/src/components/StatePage.tsx`
- Modify: `desktop/src/components/AppTabs.tsx`
- Modify: `desktop/src/App.tsx`
- Modify: `desktop/src/App.css` (append `.state-page`/`.sp-*` styles)

**Interfaces:**
- Consumes: `listState`, `setState` (Task 3); the `deck-state` Tauri event (Task 2).

- [ ] **Step 1: Create StatePage**

Create `desktop/src/components/StatePage.tsx`:

```tsx
import { listen } from "@tauri-apps/api/event";
import { useEffect, useState } from "react";
import { listState, setState } from "../api";

export default function StatePage() {
  const [values, setValues] = useState<Record<string, string>>({});
  const [newKey, setNewKey] = useState("");
  const [newVal, setNewVal] = useState("");

  useEffect(() => {
    listState().then(setValues);
    const un = listen<{ key: string; value: string }>("deck-state", (e) =>
      setValues((v) => ({ ...v, [e.payload.key]: e.payload.value })),
    );
    return () => {
      un.then((f) => f());
    };
  }, []);

  function edit(key: string, value: string) {
    setValues((v) => ({ ...v, [key]: value }));
  }

  async function addKey() {
    const k = newKey.trim();
    if (!k) return;
    await setState(k, newVal);
    setNewKey("");
    setNewVal("");
  }

  const keys = Object.keys(values).sort();
  return (
    <div className="state-page">
      <div className="sp-section">Live values</div>
      {keys.length === 0 ? (
        <p className="sp-empty">No values yet. Add one below, or push via the /kv endpoint.</p>
      ) : (
        keys.map((k) => (
          <div className="sp-row" key={k}>
            <span className="sp-key">{k}</span>
            <input className="sp-val" value={values[k]} onChange={(e) => edit(k, e.target.value)} />
            <button type="button" onClick={() => setState(k, values[k])}>
              Set
            </button>
          </div>
        ))
      )}
      <div className="sp-add">
        <input placeholder="key" value={newKey} onChange={(e) => setNewKey(e.target.value)} />
        <input placeholder="value" value={newVal} onChange={(e) => setNewVal(e.target.value)} />
        <button type="button" onClick={addKey}>
          Add
        </button>
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Add the State tab to AppTabs**

Replace the entire contents of `desktop/src/components/AppTabs.tsx` with:

```tsx
type View = "editor" | "devices" | "state";

interface Props {
  view: View;
  onSelect: (v: View) => void;
  pendingBadge: boolean;
}

export default function AppTabs({ view, onSelect, pendingBadge }: Props) {
  return (
    <div className="app-tabs">
      <button
        type="button"
        className={`app-tab ${view === "editor" ? "active" : ""}`}
        onClick={() => onSelect("editor")}
      >
        Editor
      </button>
      <button
        type="button"
        className={`app-tab ${view === "devices" ? "active" : ""}`}
        onClick={() => onSelect("devices")}
      >
        Devices
        {pendingBadge && <span className="app-tab-badge" />}
      </button>
      <button
        type="button"
        className={`app-tab ${view === "state" ? "active" : ""}`}
        onClick={() => onSelect("state")}
      >
        State
      </button>
    </div>
  );
}
```

- [ ] **Step 3: Wire the State view into App.tsx**

In `desktop/src/App.tsx`:

(a) add the import after the other component imports:

```tsx
import StatePage from "./components/StatePage";
```

(b) change the view state type:

```tsx
  const [view, setView] = useState<"editor" | "devices" | "state">("editor");
```

(c) change the render branch at the bottom — replace:

```tsx
      ) : (
        <DevicesPage
          devices={devices}
          onForget={handleForget}
          pending={pending}
          onResolved={() => setPending(null)}
        />
      )}
```

with:

```tsx
      ) : view === "devices" ? (
        <DevicesPage
          devices={devices}
          onForget={handleForget}
          pending={pending}
          onResolved={() => setPending(null)}
        />
      ) : (
        <StatePage />
      )}
```

- [ ] **Step 4: Append styles**

Append to `desktop/src/App.css`:

```css
/* State (live values) page */
.state-page {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 20px;
}
.sp-section {
  font-size: 0.7rem;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: #7a7a86;
  margin-bottom: 12px;
}
.sp-empty {
  font-size: 0.85rem;
  color: #7a7a86;
}
.sp-row {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 8px;
}
.sp-key {
  font-family: ui-monospace, Menlo, monospace;
  font-size: 0.82rem;
  color: #c4c4ce;
  min-width: 160px;
}
.sp-val {
  flex: 1;
  background: #15151a;
  border: 1px solid #2c2c34;
  border-radius: 6px;
  padding: 6px 9px;
  color: #e6e6ea;
  font-size: 0.82rem;
}
.sp-add {
  display: flex;
  gap: 8px;
  margin-top: 16px;
  border-top: 1px solid #2c2c34;
  padding-top: 16px;
}
.sp-add input {
  background: #15151a;
  border: 1px solid #2c2c34;
  border-radius: 6px;
  padding: 6px 9px;
  color: #e6e6ea;
  font-size: 0.82rem;
}
```

- [ ] **Step 5: Build**

Run: `cd desktop && npm run build`
Expected: PASS (tsc + vite).

- [ ] **Step 6: Commit**

```bash
git add desktop/src/components/StatePage.tsx desktop/src/components/AppTabs.tsx desktop/src/App.tsx desktop/src/App.css
git commit -m "feat(desktop): State tab for live values (manual source)

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 5: Desktop button editor — Binding section

**Files:**
- Modify: `desktop/src/components/ButtonEditor.tsx`
- Modify: `desktop/src/App.css` (append `.bind-editor` styles)

**Interfaces:**
- Consumes: `Bind` on `DeckButton` (Task 3).

- [ ] **Step 1: Add the Binding section**

In `desktop/src/components/ButtonEditor.tsx`, insert this block between the Color `</label>` (ends at line ~104) and the `<div className="mode-toggle">` line:

```tsx
      <div className="bind-editor">
        <label className="bind-enable">
          <input
            type="checkbox"
            checked={!!button.bind}
            onChange={(e) =>
              onChange({
                ...button,
                bind: e.target.checked ? { key: "", mode: "toggle" } : undefined,
              })
            }
          />
          Bind to a live value
        </label>
        {button.bind && (
          <div className="bind-fields">
            <label>
              Key
              <input
                value={button.bind.key}
                onChange={(e) => onChange({ ...button, bind: { ...button.bind!, key: e.target.value } })}
              />
            </label>
            <label>
              Mode
              <select
                value={button.bind.mode}
                onChange={(e) =>
                  onChange({ ...button, bind: { ...button.bind!, mode: e.target.value as "toggle" | "text" } })
                }
              >
                <option value="toggle">Toggle</option>
                <option value="text">Text</option>
              </select>
            </label>
            {button.bind.mode === "toggle" ? (
              <>
                <label>
                  On value
                  <input
                    value={button.bind.on_value ?? ""}
                    placeholder="1"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_value: e.target.value } })}
                  />
                </label>
                <label>
                  Off value
                  <input
                    value={button.bind.off_value ?? ""}
                    placeholder="0"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_value: e.target.value } })}
                  />
                </label>
                <label>
                  On label
                  <input
                    value={button.bind.on_label ?? ""}
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_label: e.target.value } })}
                  />
                </label>
                <label>
                  Off label
                  <input
                    value={button.bind.off_label ?? ""}
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_label: e.target.value } })}
                  />
                </label>
                <label>
                  On color
                  <input
                    value={button.bind.on_color ?? ""}
                    placeholder="#F87171"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_color: e.target.value } })}
                  />
                </label>
                <label>
                  Off color
                  <input
                    value={button.bind.off_color ?? ""}
                    placeholder="#3A3A44"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_color: e.target.value } })}
                  />
                </label>
              </>
            ) : (
              <label>
                Format
                <input
                  value={button.bind.format ?? ""}
                  placeholder="{}"
                  onChange={(e) => onChange({ ...button, bind: { ...button.bind!, format: e.target.value } })}
                />
              </label>
            )}
          </div>
        )}
      </div>
```

- [ ] **Step 2: Append styles**

Append to `desktop/src/App.css`:

```css
/* Button editor — live-value binding */
.bind-editor {
  border-top: 1px solid #2c2c34;
  padding-top: 12px;
  margin-top: 4px;
}
.bind-enable {
  flex-direction: row !important;
  align-items: center;
  gap: 8px;
  font-size: 0.85rem;
}
.bind-fields {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-top: 8px;
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS.

- [ ] **Step 4: Commit**

```bash
git add desktop/src/components/ButtonEditor.tsx desktop/src/App.css
git commit -m "feat(desktop): bind a button to a live value in the editor

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 6: Firmware config — parse the `bind` field

**Files:**
- Modify: `src/core/deck_config.h`
- Modify: `src/core/deck_config.cpp`

**Interfaces:**
- Produces: `deck_config::bind_t` + `button.bind` (consumed by Tasks 7-8).

- [ ] **Step 1: Add the bind struct to the model**

In `src/core/deck_config.h`, add before `struct button {`:

```cpp
struct bind_t {
  bool present = false;
  String key;
  String mode = "toggle";   // "toggle" | "text"
  String on_value = "1";
  String off_value = "0";
  String on_label;
  String off_label;
  String on_color;
  String off_color;
  String format;
};
```

And add a field to `struct button`:

```cpp
struct button {
  int pos = -1;
  String label;
  String icon;       // filename under /deck/icons/, or empty
  String color;      // "#RRGGBB" or empty
  String open_page;  // folder target page id, or empty
  bind_t bind;       // live-value binding (present=false if absent)
  std::vector<step> steps;
};
```

- [ ] **Step 2: Parse it**

In `src/core/deck_config.cpp`, inside the `for (JsonObjectConst b : p["buttons"]...)` loop, after the `btn.open_page = ...;` line and before the steps loop, add:

```cpp
      JsonObjectConst bd = b["bind"];
      if (!bd.isNull()) {
        btn.bind.present = true;
        btn.bind.key = (const char *)(bd["key"] | "");
        btn.bind.mode = (const char *)(bd["mode"] | "toggle");
        btn.bind.on_value = (const char *)(bd["on_value"] | "1");
        btn.bind.off_value = (const char *)(bd["off_value"] | "0");
        btn.bind.on_label = (const char *)(bd["on_label"] | "");
        btn.bind.off_label = (const char *)(bd["off_label"] | "");
        btn.bind.on_color = (const char *)(bd["on_color"] | "");
        btn.bind.off_color = (const char *)(bd["off_color"] | "");
        btn.bind.format = (const char *)(bd["format"] | "");
      }
```

- [ ] **Step 3: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles (the new field is unused until Tasks 7-8 — acceptable).

- [ ] **Step 4: Commit**

```bash
git add src/core/deck_config.h src/core/deck_config.cpp
git commit -m "feat(fw): parse button live-value bind config

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 7: Firmware — deck_state cache + deck_client wiring

**Files:**
- Create: `src/core/deck_state.h`, `src/core/deck_state.cpp`
- Modify: `src/core/deck_client.h`, `src/core/deck_client.cpp`
- Modify: `src/main.cpp` (call `deck_state::init()` in setup)

**Interfaces:**
- Produces: `deck_state::{init,set_local,get,has,set_and_send,consume_changed}`; `deck_client::send_set(key, value)`. Consumed by Task 8.

- [ ] **Step 1: Create deck_state.h**

Create `src/core/deck_state.h`:

```cpp
#pragma once

#include <Arduino.h>

// In-memory cache of the desktop's live key/value bus (protocol §3.5). Written
// from the deck_client WS task (inbound state/state_snapshot) and read from the
// LVGL thread (tile rendering), so all access is mutex-guarded.
namespace deck_state {

// Creates the guard mutex. Call once from setup(), before deck_client::start().
void init();

// Applies an inbound value (from `state`/`state_snapshot`); marks changed.
void set_local(const String &key, const String &value);

// Current value for a key, or "" if unknown.
String get(const String &key);

// True if the key has a cached value.
bool has(const String &key);

// Updates the cache locally and sends a `set` to the desktop (toggle tap).
void set_and_send(const String &key, const String &value);

// True once per change since the last call (LVGL thread polls this to re-render).
bool consume_changed();

} // namespace deck_state
```

- [ ] **Step 2: Create deck_state.cpp**

Create `src/core/deck_state.cpp`:

```cpp
#include "core/deck_state.h"

#include <map>

#include "core/deck_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace deck_state {

static std::map<String, String> s_map;
static SemaphoreHandle_t s_mtx = nullptr;
static volatile bool s_changed = false;

void init()
{
  if (!s_mtx) s_mtx = xSemaphoreCreateMutex();
}

void set_local(const String &key, const String &value)
{
  if (!s_mtx || key.isEmpty()) return;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  s_map[key] = value;
  xSemaphoreGive(s_mtx);
  s_changed = true;
}

String get(const String &key)
{
  String out;
  if (!s_mtx) return out;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  auto it = s_map.find(key);
  if (it != s_map.end()) out = it->second;
  xSemaphoreGive(s_mtx);
  return out;
}

bool has(const String &key)
{
  if (!s_mtx) return false;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  bool h = s_map.find(key) != s_map.end();
  xSemaphoreGive(s_mtx);
  return h;
}

void set_and_send(const String &key, const String &value)
{
  set_local(key, value);
  deck_client::send_set(key, value);
}

bool consume_changed()
{
  if (!s_changed) return false;
  s_changed = false;
  return true;
}

} // namespace deck_state
```

- [ ] **Step 3: Declare send_set in deck_client.h**

In `src/core/deck_client.h`, add inside `namespace deck_client` (e.g. after `bool consume_icons_pushed();`):

```cpp
// Sends {"t":"set","key":...,"value":...} (protocol §3.5) when a toggle-bound
// tile is tapped. No-op if not connected/authed.
void send_set(const String &key, const String &value);
```

- [ ] **Step 4: Implement send_set + handle inbound state in deck_client.cpp**

In `src/core/deck_client.cpp`, add the include near the others:

```cpp
#include "core/deck_state.h"
```

In `handle_message`, add branches to the if/else chain (after the `pair_rejected` branch, before the trailing comment):

```cpp
  } else if (!strcmp(t, "state_snapshot")) {
    for (JsonPairConst kv : doc["values"].as<JsonObjectConst>()) {
      deck_state::set_local(kv.key().c_str(), (const char *)(kv.value() | ""));
    }
  } else if (!strcmp(t, "state")) {
    deck_state::set_local((const char *)(doc["key"] | ""),
                          (const char *)(doc["value"] | ""));
```

Add the `send_set` implementation before the closing `} // namespace deck_client`:

```cpp
void send_set(const String &key, const String &value)
{
  if (!s_authed || !s_ws.isConnected() || key.isEmpty()) return;
  JsonDocument doc;
  doc["t"] = "set";
  doc["key"] = key;
  doc["value"] = value;
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}
```

- [ ] **Step 5: Init the cache in setup()**

In `src/main.cpp`, add the include near the others:

```cpp
#include "core/deck_state.h"
```

In `setup()`, add `deck_state::init();` on the line immediately before `deck_client::start();`:

```cpp
    deck_state::init();
    deck_client::start();
```

- [ ] **Step 6: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles clean.

- [ ] **Step 7: Commit**

```bash
git add src/core/deck_state.h src/core/deck_state.cpp src/core/deck_client.h src/core/deck_client.cpp src/main.cpp
git commit -m "feat(fw): deck_state cache + state_snapshot/state/set wiring

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 8: Firmware — render bound tiles, toggle-on-tap, live refresh

**Files:**
- Modify: `src/apps/deck/deck.h`, `src/apps/deck/deck.cpp`
- Modify: `src/main.cpp` (poll `consume_changed`)

**Interfaces:**
- Consumes: `deck_config::button.bind` (Task 6); `deck_state::{get,set_and_send,consume_changed}` (Task 7).

- [ ] **Step 1: Expose a binding-refresh entry point**

In `src/apps/deck/deck.h`, add after `void deck_invalidate_config();`:

```cpp
// Re-renders the current grid so tiles bound to live values reflect the latest
// deck_state cache. Cheap (no SD reload). Call from the LVGL thread (main loop)
// after observing deck_state::consume_changed(). No-op if the screen is closed.
void deck_refresh_bindings();
```

- [ ] **Step 2: Add the bind include + render helper in deck.cpp**

In `src/apps/deck/deck.cpp`, add the include near the others:

```cpp
#include "core/deck_state.h"
```

Add this helper above `make_tile`:

```cpp
// Computes a tile's display label and background color, applying any live-value
// binding. For an unbound button (or a bound key with no value yet) it returns
// the static label and the static color.
static String tile_label_and_color(const deck_config::button &btn, lv_color_t &out_color)
{
  out_color = tile_color(btn.color);
  if (!btn.bind.present || btn.bind.key.isEmpty()) return btn.label;

  String val = deck_state::get(btn.bind.key);
  if (btn.bind.mode == "text") {
    if (val.isEmpty()) return btn.label;
    if (btn.bind.format.isEmpty()) return val;
    String s = btn.bind.format;
    s.replace("{}", val);
    return s;
  }

  // toggle
  bool on = (val == btn.bind.on_value);
  if (on && btn.bind.on_color.length()) out_color = tile_color(btn.bind.on_color);
  else if (!on && btn.bind.off_color.length()) out_color = tile_color(btn.bind.off_color);
  String lbl = on ? btn.bind.on_label : btn.bind.off_label;
  return lbl.length() ? lbl : btn.label;
}
```

- [ ] **Step 3: Use the helper in make_tile**

In `make_tile`, replace:

```cpp
  lv_obj_set_style_bg_color(tile, tile_color(btn.color), 0);
```

with:

```cpp
  lv_color_t tile_bg;
  String tile_text = tile_label_and_color(btn, tile_bg);
  lv_obj_set_style_bg_color(tile, tile_bg, 0);
```

and replace:

```cpp
  lv_label_set_text(label, btn.label.c_str());
```

with:

```cpp
  lv_label_set_text(label, tile_text.c_str());
```

- [ ] **Step 4: Toggle-on-tap in tile_click_cb**

In `tile_click_cb`, replace:

```cpp
  if (btn.open_page.length()) {
    s_page_stack.push_back(btn.open_page);
    rebuild_grid();
  } else {
    deck_executor::run(s_cfg, btn);
  }
```

with:

```cpp
  if (btn.open_page.length()) {
    s_page_stack.push_back(btn.open_page);
    rebuild_grid();
  } else if (btn.bind.present && btn.bind.mode == "toggle" && btn.bind.key.length()) {
    String cur = deck_state::get(btn.bind.key);
    bool on = (cur == btn.bind.on_value);
    deck_state::set_and_send(btn.bind.key, on ? btn.bind.off_value : btn.bind.on_value);
    rebuild_grid(); // optimistic: reflect the flip immediately
  } else {
    deck_executor::run(s_cfg, btn);
  }
```

- [ ] **Step 5: Implement deck_refresh_bindings**

In `src/apps/deck/deck.cpp`, add next to `deck_invalidate_config` (it can call the existing static `rebuild_grid`):

```cpp
void deck_refresh_bindings()
{
  if (s_root) rebuild_grid();
}
```

- [ ] **Step 6: Poll for state changes in the main loop**

In `src/main.cpp`'s `loop()`, after the existing config/icons block:

```cpp
  if (deck_client::consume_config_pushed() || deck_client::consume_icons_pushed()) {
    deck_invalidate_config();
  }
```

add:

```cpp
  if (deck_state::consume_changed()) {
    deck_refresh_bindings();
  }
```

- [ ] **Step 7: Build**

Run: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35`
Expected: compiles clean.

- [ ] **Step 8: Manual end-to-end verification**

Flash + run the desktop: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35 -t upload -t monitor` and `cd desktop && npm run tauri dev`. Author a config with two bound tiles (one `toggle` on key `mic_muted` with on/off labels+colors; one `text` on key `cpu_temp` with format `{}°C`), save/push. Then:
1. **Push down (UI):** Desktop → State tab → add `cpu_temp` = `62` → the text tile shows `62°C`. Change it → tile updates live. Set `mic_muted` = `1` → the toggle tile flips to its on look.
2. **Push down (HTTP):** `curl "http://localhost:<port>/kv/set?key=cpu_temp&value=70"` (port from the `ws_server: listening on port N` log) → tile shows `70°C`.
3. **Tap up:** tap the toggle tile on the device → it flips instantly; the State tab's `mic_muted` updates; a second connected device's toggle tile reflects it too.
4. **Snapshot:** power-cycle the device → on reconnect both bound tiles show the current values immediately (not the static fallback).

- [ ] **Step 9: Commit**

```bash
git add src/apps/deck/deck.h src/apps/deck/deck.cpp src/main.cpp
git commit -m "feat(fw): render live-value tiles + toggle-on-tap + state refresh

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Notes on testing strategy

Only the desktop store/snapshot helper has `cargo test` coverage (Task 2). Everything else is build-gated (`cargo build` / `npm run build` / `pio run`) plus the Task 8 manual E2E pass, which exercises the full loop in both directions (source→tile and tap→hub→all devices) and the snapshot-on-reconnect path from the spec §7.
