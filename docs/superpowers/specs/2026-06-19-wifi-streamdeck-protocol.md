# WiFi Stream Deck — Phase 0: Config Schema & WebSocket Protocol

The wire contract shared by the ESP32 deck firmware and the Tauri desktop app.
Companion to [2026-06-19-wifi-streamdeck-design.md](2026-06-19-wifi-streamdeck-design.md).
This doc is the single source of truth for the config JSON and the WS messages;
both sides implement against it.

## 1. Config JSON

Stored on the device at `/deck/config.json`; authored/owned by the desktop app
and pushed over the WebSocket. The desktop app keeps the canonical copy.

```jsonc
{
  "version": 1,                       // schema version (integer; bump on breaking change)
  "grid": { "cols": 3, "rows": 4 },   // tiles per page
  "agent": {
    "token": "8-char-pairing-secret"  // must match to authenticate the WS
  },
  "ha": {                              // optional; required only if ha_* steps are used
    "base_url": "http://homeassistant.local:8123",
    "token": "long-lived-access-token"
  },
  "pages": [
    {
      "id": "home",                    // unique page id; "home" is the entry page
      "title": "Home",
      "buttons": [
        {
          "pos": 0,                    // 0-based slot index, 0..(cols*rows-1)
          "label": "Standup",
          "icon": "standup.bin",       // filename under /deck/icons/, or omitted
          "color": "#3ECF8E",          // tile accent/background tint (optional)
          "steps": [ /* see §2 */ ]
        },
        {
          "pos": 1,
          "label": "Work",
          "icon": "folder.bin",
          "open_page": "work"          // folder button: navigates, ignores steps
        }
      ]
    }
  ]
}
```

Rules:
- A button has **either** `steps` **or** `open_page` (folder). If both are
  present, `open_page` wins.
- Missing `icon` → the device renders the `label` text tile only.
- `pos` values within a page are unique; gaps are allowed (empty slots).
- `id` `"home"` is the required entry page.

## 2. Step objects

`steps` is an ordered array; the device executes them top→bottom, stopping on a
fatal error (a failed step reports back but later steps still run — see §4).
Every step has a `"type"`; other fields depend on the type.

### Direct steps (device performs over the network)

```jsonc
{ "type": "http_request",
  "method": "POST",                    // GET|POST|PUT|DELETE
  "url": "http://homeserver.local/api/x",
  "headers": { "Content-Type": "application/json" },
  "body": "{\"on\":true}" }            // string; omitted for GET

{ "type": "ha_service",
  "domain": "light", "service": "toggle",
  "data": { "entity_id": "light.office" } }   // POST {base}/api/services/{domain}/{service}

{ "type": "ha_webhook",
  "id": "office_scene" }               // POST {base}/api/webhook/{id}

{ "type": "mqtt_publish",              // schema-only in v1; implemented Phase 4
  "topic": "home/deck/btn1", "payload": "ON" }
```

### Host steps (agent performs; device sends `exec`, see §3)

```jsonc
{ "type": "hotkey",   "keys": ["cmd","shift","a"] }   // modifier+key chord
{ "type": "type_text","text": "hello world" }
{ "type": "launch_app","target": "zoom.us" }          // app name/path/bundle id
{ "type": "run_command","command": "/usr/local/bin/backup.sh" }
{ "type": "media_key","key": "play_pause" }           // play_pause|next|prev|vol_up|vol_down|mute
```

### Control step (device)

```jsonc
{ "type": "delay", "ms": 1500 }
```

Step `type` registry (v1): `http_request`, `ha_service`, `ha_webhook`,
`mqtt_publish`, `hotkey`, `type_text`, `launch_app`, `run_command`,
`media_key`, `delay`.

## 3. WebSocket protocol

- **Roles:** desktop app = WS **server**; device = WS **client**.
- **Discovery:** desktop advertises mDNS service `_deckhost._tcp` (TXT may carry
  `ws_path`, default `/ws`); device browses, resolves host:port, connects.
- **Encoding:** text frames carry JSON messages (below). Binary frames carry
  icon payloads (§3.3). Every message object has a `"t"` (type) field.

### 3.1 Handshake / auth
```jsonc
// device → app, first message after connect
{ "t": "hello", "device_id": "esp32-aabbcc", "fw": "1.0.0", "token": "ABC12345" }
// app → device
{ "t": "auth_ok" }                     // token matched
{ "t": "auth_fail", "reason": "bad token" }   // app then closes the socket
```
No other message is honored before `auth_ok`. `run_command`/`launch_app` `exec`
requests are rejected by the app if the connection is unauthenticated.

### 3.2 Config push (app → device)
```jsonc
{ "t": "config", "config": { /* full config JSON, §1 */ }, "rev": 7 }
// device → app, after persisting to SD and rebuilding the grid
{ "t": "config_ack", "rev": 7 }
```
Config is always sent whole (not diffed). The device persists then rebuilds.

### 3.3 Icon transfer (app → device)
For each icon the config references that the device lacks:
```jsonc
{ "t": "icon_begin", "name": "standup.bin", "bytes": 16200, "w": 90, "h": 90 }
```
…immediately followed by one or more **binary** WS frames totaling `bytes`
(RGB565, row-major). Then:
```jsonc
{ "t": "icon_end", "name": "standup.bin" }
// device → app
{ "t": "icon_ack", "name": "standup.bin" }
```
The device may advertise which icons it already has so the app skips them:
```jsonc
{ "t": "have_icons", "names": ["folder.bin","mute.bin"] }   // device → app, after auth
```

### 3.4 Host-step execution (device → app)
```jsonc
{ "t": "exec", "id": 42, "step": { "type":"hotkey", "keys":["cmd","shift","a"] } }
// app → device
{ "t": "result", "id": 42, "ok": true }
{ "t": "result", "id": 42, "ok": false, "error": "no accessibility permission" }
```
`id` is a device-assigned monotonic request id so results match requests. The
device waits (with a timeout, ~3 s) for the matching `result` before running the
next step in the sequence.

### 3.5 Live state (Phase 4, reserved)
```jsonc
{ "t": "state", "page": "home", "pos": 0, "value": { "on": true } }  // app → device
```
Reserved now so v1 implementations ignore unknown `t` values forward-compatibly.

## 4. Execution semantics

- The device runs a button's `steps` in order on a worker task (off the LVGL
  thread).
- `delay` sleeps the worker; direct steps block on HTTP with a per-step timeout
  (~3 s); host steps send `exec` and await `result` (~3 s timeout).
- A failed/timed-out step is recorded (and surfaced in the status bar / a toast)
  but the sequence **continues** to the next step. (Rationale: a deck tap should
  be best-effort; a stricter "stop on error" mode can be a later config flag.)
- Re-tapping a button while its sequence is still running is ignored until it
  finishes.

## 5. Versioning & compatibility

- `config.version` and an mDNS/`hello` `fw` let either side detect mismatches.
- Unknown step `type`s and unknown message `t`s are **ignored, not fatal**, so a
  newer app can talk to an older device (the step is skipped, logged).
- Breaking changes bump `config.version`; the app refuses to push a config newer
  than the device understands and prompts to update firmware.
