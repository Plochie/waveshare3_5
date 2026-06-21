# WiFi Stream Deck — Live State Bus (Bidirectional Key/Value)

A bidirectional key/value bus between the desktop hub and the deck device:
the desktop holds live named values, pushes them to devices so tiles reflect
them, and a tile tap can set a value back — keeping every device consistent.
This is the foundation the deferred "live/bidirectional button state" (Phase 4)
builds on.

Companion to the protocol
[2026-06-19-wifi-streamdeck-protocol.md](2026-06-19-wifi-streamdeck-protocol.md)
(this spec replaces the reserved §3.5 `state` message) and the system design
[2026-06-19-wifi-streamdeck-design.md](2026-06-19-wifi-streamdeck-design.md).
Pairing, config sync, icons, and host-action exec already ship; this adds the
live-state channel on top.

## Scope

**v1 = the bus + tile binding + two trivial sources (manual UI-set and a
programmatic HTTP setter).** Host-system-state and Home Assistant adapters are
deliberately **out of scope** for this spec — they are independent source
adapters that plug into the finished bus in their own later cycles. Building the
bus first yields a complete, end-to-end testable feature (tiles reflect values
you push; a tap toggles a value and every device updates) without any external
integration work.

## 1. Architecture & data model

A **key/value bus** with the desktop as the single authoritative hub:

- The desktop holds an **in-memory store**: `key -> value`. Keys are flat
  free-form strings (`mic_muted`, `cpu_temp`, `office_light`); values are
  strings on the wire (a tile interprets the string per its bind mode).
- **Sources** write into the store via one `set_value(key, value)` path (v1: the
  manual UI panel and the programmatic HTTP endpoint). The store is
  source-agnostic.
- On any change, the desktop **broadcasts** the new value to every connected,
  authenticated device, and notifies its own UI.
- A device **caches** keys it has seen; on connect it receives a **snapshot** of
  the whole store so bound tiles render immediately.
- A **tile binds one key** and renders it (toggle or text). A `toggle` tile's
  tap **sends a set** (the device flips its cached value and reports the new
  one); the hub stores it and echoes to all devices, so they stay consistent.
- **Persistence:** the store is live/in-memory only (this is *live state*). It
  is not saved across desktop restarts. On restart the store is empty; devices
  reconnect, receive an empty snapshot, and bound tiles fall back to their
  static label/color until a source re-feeds values.

Rationale: one clean primitive (a string key/value bus) drives tiles now and
extends to conditionals/variables later; desktop-as-hub means multiple devices
never disagree.

## 2. Protocol (replaces reserved §3.5)

Three message types. Values are always strings; unknown `t` values and unknown
keys are ignored (forward-compatible, as today).

**Snapshot — app→device** (sent right after `auth_ok` + config push):
```jsonc
{ "t": "state_snapshot", "values": { "mic_muted": "1", "cpu_temp": "62" } }
```

**Delta — app→device** (sent to every connected device on each change):
```jsonc
{ "t": "state", "key": "mic_muted", "value": "0" }
```

**Set — device→app** (sent when a toggle tile is tapped):
```jsonc
{ "t": "set", "key": "mic_muted", "value": "0" }
```
On receipt the desktop stores the value and broadcasts the matching `state`
delta to **all** devices including the sender (idempotent; keeps everyone
consistent). `set` is honored only on an authenticated connection, like `exec`.

No per-key subscription in v1: a device receives all keys and ignores those no
tile binds (key counts are tiny). Concurrent writes (a device `set` and a source
change) resolve last-write-wins at the hub, which serializes them; the broadcast
echo is the final truth.

## 3. Config schema — tile binding

A button gains an optional `bind` object (alongside `label`/`icon`/`color`/
`steps`/`open_page`):

```jsonc
"bind": {
  "key": "mic_muted",        // bus key driving this tile
  "mode": "toggle",          // "toggle" | "text"

  // toggle mode:
  "on_value":  "1",          // value treated as "on"  (default "1")
  "off_value": "0",          // value sent/treated as "off" (default "0")
  "on_label":  "Muted",      // label when on  (optional; falls back to button label)
  "off_label": "Mic",        // label when off (optional)
  "on_color":  "#F87171",    // tile tint when on  (optional; falls back to button color)
  "off_color": "#3A3A44",    // tile tint when off (optional)

  // text mode:
  "format": "{}°C"           // {} replaced by the value (optional; default the bare value)
}
```

**Rendering**
- `toggle`: "on" when the cached value equals `on_value`; uses on/off label +
  color, each falling back to the static `label`/`color` when unspecified or
  before any value has arrived.
- `text`: tile label = `format` with `{}` -> value; falls back to the static
  `label` until a value arrives.

**Tap behavior**
- `toggle`: tap **sends a `set`** — flips to `off_value` when currently on, else
  `on_value` (defaults to `on_value` when nothing is cached). Does not run
  `steps`.
- `text`: display-only by default; if the button also has `steps`, tap runs them
  (a readout can double as an action button).
- `bind` and `open_page` are mutually exclusive; if both appear `open_page`
  wins, consistent with the existing folder rule.

## 4. Desktop implementation

Reuses the existing axum WS server + `ServerState` (`desktop/src-tauri/`).

- **Store:** add `state: Mutex<HashMap<String,String>>` to `ServerState`. A
  single `set_value(key, value)` helper updates the store, broadcasts a `state`
  delta to all authenticated clients, and emits a `deck-state` Tauri event for
  the app UI. Every source funnels through it.
- **Snapshot on connect:** after the existing `auth_ok` + config push in
  `handle_socket`, send `state_snapshot` with the current store.
- **Device `set` handler:** new `InMsg::Set { key, value }` in the post-auth
  receive loop calls `set_value` (stores + re-broadcasts to all, including the
  sender).
- **Manual source (UI):** a new **State** top-level tab (alongside Editor |
  Devices, reusing the tab shell), listing current `key -> value` pairs with
  add/edit/set controls, live-updated from the `deck-state` event (so
  device-originated toggles appear). A `set_state(key, value)` Tauri command
  calls `set_value`.
- **Programmatic source (HTTP):** add routes to the existing axum server —
  `POST /kv` with `{ "key", "value" }` and a convenience
  `GET /kv/set?key=&value=` — each calling `set_value`. Reuses the WS server's
  port; localhost-trust model, consistent with today.
- **Authoring:** the button editor gains a "Binding" section (key + mode +
  mode-specific fields); `bind` rides in the config JSON already pushed and
  persisted.

Pure helpers (the store mutation + the value/format interpretation that the UI
shares) get Rust `#[cfg(test)]` unit tests.

## 5. Firmware implementation

- **`deck_state` module** (`src/core/deck_state.{h,cpp}`, new): a guarded cache
  (`std::map<String,String>` behind a mutex). API: `set_local(key, value)` (from
  inbound messages), `get(key) -> String`, `has(key)`, `set_and_send(key,
  value)` (updates the cache and sends a `set` via `deck_client`), and
  `consume_changed()` (edge flag polled by the UI thread).
- **`deck_client`:** handle inbound `state_snapshot` (load all into
  `deck_state`) and `state` (update one key), marking `deck_state` changed; add
  `send_set(key, value)` used by `deck_state::set_and_send`. Mirrors the existing
  config/icon handling and runs on the WS task.
- **`deck.cpp`:** when a tile's `btn.bind` is present, render per mode from
  `deck_state::get(bind.key)` (toggle compares to `on_value` for label/color;
  text fills `format`), with fallback to the static label/color. The tap
  handler: for a `toggle` bind, compute the flipped value and call
  `deck_state::set_and_send` (optimistically updating the cache so the tile
  flips instantly) instead of running steps.
- **Live refresh:** `main.cpp`'s loop polls `deck_state::consume_changed()`
  (next to the existing config/icon polls) and triggers a lightweight re-render
  of the current grid (reusing the existing rebuild path). Deltas are applied to
  the cache on the `deck_client` task; the LVGL thread only reads the cache and
  re-renders — matching the existing cross-thread pattern.

`deck_config` parsing gains a `bind` struct on the `button` model.

## 6. Edge cases

- **No value yet for a bound key** — tile shows its static label/color fallback.
- **Toggle tap with nothing cached** — sends `on_value` (turns on).
- **Device reconnect** — the snapshot re-syncs all bound tiles.
- **Two devices toggle near-simultaneously** — the hub serializes; last write
  wins; the broadcast echo converges every device.
- **`set` for a key no source manages** — stored anyway (the bus is free-form;
  device-owned keys are valid).
- **Desktop restart** — store empties; on reconnect devices get an empty
  snapshot and bound tiles fall back to static until sources re-feed.
- **Unknown `t` / unknown key** — ignored (forward-compat).

## 7. Testing

Manual (no firmware/JS harness) plus Rust unit tests for the store/format
helpers:

1. **Push down** — set a key via the State tab and via `curl` the HTTP endpoint;
   a `text`-bound tile shows the value, a `toggle`-bound tile flips its
   look. Change it again → tile updates live.
2. **Tap up** — tap a `toggle` tile on the device → its look flips instantly,
   the desktop State tab shows the new value, and a second connected device's
   tile reflects it too.
3. **Snapshot** — power-cycle the device → on reconnect its bound tiles render
   the current values immediately (not the static fallback).
4. **Fallback** — a bound key with no value yet shows the static label/color;
   after a value arrives it switches.
5. **Forward-compat** — confirm an old-firmware device ignores `state`/
   `state_snapshot` without breaking.

## 8. Out of scope (later cycles / phases)

- **Host-system-state source** (mute/volume/now-playing watchers) — its own
  cycle; plugs into `set_value`.
- **Home Assistant source** (subscribe/poll HA entities -> keys) — its own cycle.
- Conditionals / computed values / variables in workflows.
- Value types beyond strings; templating beyond a single `{}`.
- Per-key subscription; store persistence across desktop restart.
- `mqtt_publish`, TLS/secret hardening (separate Phase 4 items).
