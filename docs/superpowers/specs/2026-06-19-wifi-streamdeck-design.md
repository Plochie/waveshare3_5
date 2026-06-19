# WiFi Stream Deck — System Design

## Purpose

Turn the Waveshare ESP32-S3-Touch-LCD-3.5B into a Stream Deck-style controller:
a configurable grid of icon buttons where each tap runs a **multi-step
workflow**. Workflows mix actions the device performs itself over the network
(Home Assistant, generic HTTP APIs, a home server) with actions performed on a
host machine through a companion desktop app (hotkeys, type text, launch apps,
run scripts, media keys). A cross-platform **desktop configurator** designs the
grid and pushes config + icons to the device.

This is the umbrella design. The wire contract (config JSON schema + WebSocket
message protocol) is specified separately in
[2026-06-19-wifi-streamdeck-protocol.md](2026-06-19-wifi-streamdeck-protocol.md).

## Prior art (research summary)

- **Elgato Stream Deck** — the device is "dumb"; desktop software runs the
  actions; plugins talk to the app over **WebSocket + JSON**; each action has a
  config UI ("property inspector").
  (https://docs.elgato.com/sdk/plugins/architecture)
- **FreeTouchDeck / ESP-Streaming-Deck** — closest ESP32 prior art. Either act
  as a **BLE/USB HID keyboard** (no host software, keystrokes only) or fire
  **MQTT/REST** over WiFi; config via a web page served from the ESP32.
  (https://github.com/DustinWatts/FreeTouchDeck,
  https://github.com/espzav/ESP-Streaming-Deck)
- **Macro Deck / Touch Portal / Deckboard** — the model for rich desktop
  actions: a **PC agent** runs hotkeys/launches/scripts; the device is a thin
  client. Macro Deck is open-source (C#/.NET). (https://macro-deck.app/)
- **Host actions need an agent** — keystrokes/launch/scripts can't be done by
  the ESP32 over WiFi; a host process is required (e.g. Rust `enigo`, or Python
  `pynput`). (https://pynput.com/)
- **Home Assistant** — reachable directly from the ESP32 via REST on `:8123`
  with a **long-lived bearer token**, or via **webhooks** (no auth).
  (https://developers.home-assistant.io/docs/auth_api/)

## Decisions locked during brainstorming

- **v1 action targets:** Home Assistant, generic HTTP APIs, desktop
  hotkeys/macros, and launch-apps/run-scripts — i.e. **both** direct-network
  and host-agent action categories.
- **Desktop OS coverage:** macOS + Windows + Linux from a **single codebase**.
- **Desktop framework:** **Tauri** (Rust core + web UI) — lightweight single
  binary, low RAM for an always-on tray agent, `enigo` for synthetic input.
- **Transport:** one **persistent WebSocket**, device → agent (Stream Deck
  style). Config + icons pushed down it; triggers/state up it. Direct-network
  actions bypass it.
- **Per-button behavior:** **multi-step linear sequences** from day one (no
  branching/conditionals in v1).

## Components

### A. ESP32 Deck firmware (this repo, new `apps/deck`)
- Config-driven LVGL grid (3×4 = 12 button tiles/page: icon + label), page
  navigation (folders), status bar (WiFi + agent-connected indicator + page
  dots). **Mostly code-built, not EEZ-designed** (the grid is dynamic).
- Storage on SD: `/deck/config.json` + `/deck/icons/*.bin`.
- **WebSocket client** (`links2004/arduinoWebSockets`): receives config/icon
  pushes (saves to SD, rebuilds grid), sends host-step `exec` requests.
- **Step executor** runs off the UI thread (FreeRTOS task, like the WLED scan)
  so blocking HTTP and `delay` steps don't stall LVGL. Direct steps via
  `HTTPClient`; host steps via the WS.
- Reuses existing infra: mDNS, HTTPClient, ArduinoJson, SD HAL, screen_manager.

### B. Desktop configurator + agent (new, Tauri)
- **Rust core:** WS **server** (`axum`/`tokio-tungstenite`), mDNS advertise
  (`mdns-sd`), input simulation (`enigo`), launch/scripts (`std::process`),
  media keys, icon processing (`image` crate), config persistence
  (`serde_json`), system tray (`tray-icon`).
- **Web UI (React or Svelte):** drag-drop grid editor, per-button step editor
  (add/reorder/configure steps), icon upload/picker, device pairing screen,
  connection status.
- **One binary** = configurator UI **and** background agent (the Macro Deck
  model). Network actions keep working when the app is closed; host actions and
  reconfiguration require it running.

### C. The contract (Phase 0)
- The config JSON schema (pages → buttons → ordered steps) and the WebSocket
  message protocol. Fully specified in the protocol doc.

## Discovery & connection

The **desktop agent advertises via mDNS** (e.g. `_deckhost._tcp`); the device
**browses** for it (reusing the mDNS query code built for WLED) and opens the
WebSocket. The device is the WS client; the desktop is the WS server.

## Icon pipeline

The **desktop pre-renders** each icon: resize to the exact tile size and
convert to an **RGB565 LVGL `.bin`**, shipped to the device, which blits it
directly — no PNG decoder on the ESP32. Re-render occurs if the grid size
changes. (Fallback option: on-device `PNGdec` to drop PNGs straight onto SD.)

## Security

`run_command` / `launch_app` let the agent execute **arbitrary code**, so an
unauthenticated LAN server here is remote code execution. Mitigations:
- **Pairing token** generated by the desktop app, shown as a code, stored on
  the device; every WS connection must `auth` with it before any `exec`.
- (Later) optional command allowlist.
- HA tokens / pairing secret live in the SD config in plaintext (local-only
  device) — TLS and secret encryption are Phase 4 hardening.

## Phased delivery (each phase ships something usable)

- **Phase 0 — Protocol & schema.** The config JSON + WS contract (the protocol
  doc). No runtime code.
- **Phase 1 — Deck firmware, direct-network steps only.** Grid UI + page nav +
  executor; steps limited to `http_request` / `ha_service` / `ha_webhook`;
  config hand-placed on SD. A working network deck with **zero desktop
  software**. Best fit for the existing C/LVGL codebase.
- **Phase 2 — Tauri app skeleton + sync.** WS server, mDNS advertise, pairing,
  push config + icons; device gains its WS client + icon rendering. Author in
  the app instead of hand-editing JSON.
- **Phase 3 — Host actions.** Agent executes `hotkey` / `type_text` /
  `launch_app` / `run_command` / `media_key`; device gains the host-step path
  over the WS; pairing enforced.
- **Phase 4 — Polish.** Live button state (button reflects mute/HA state),
  `mqtt_publish`, conditionals, TLS/secret hardening.

## Step types (v1)

- **Direct (device fires):** `http_request` (method/url/headers/body),
  `ha_service` (entity+service+data), `ha_webhook` (id),
  `mqtt_publish` *(schema-only in v1, implemented Phase 4)*.
- **Host (via agent):** `hotkey` (key combo), `type_text`, `launch_app`,
  `run_command`, `media_key`.
- **Control:** `delay` (ms).
- **Navigation:** a button with `open_page` acts as a folder (not a step).

## Risks / things to watch

- **WebSocket lib + RAM** on the ESP32 (`arduinoWebSockets` footprint atop the
  existing LVGL double buffers / PSRAM budget).
- **Non-blocking executor**: sequences contain `delay` + blocking HTTP; must run
  off the LVGL thread to keep the UI responsive (FreeRTOS task pattern already
  proven by the WLED mDNS scan).
- **Icon memory**: 12 RGB565 tiles/page at ~90×90 ≈ 16 KB each; load
  per-page, free on page change.
- **Security**: pairing is mandatory before host actions ship (Phase 3).
- **Cross-platform host actions**: macOS needs Accessibility permission for
  synthetic input; Linux differs across X11/Wayland — handled in the Rust agent
  via `enigo` with documented per-OS setup.

## Out of scope (v1)

- Branching/conditional workflows, variables, templating.
- Live/bidirectional button state (Phase 4).
- TLS / encrypted secret storage (Phase 4).
- Plugin ecosystem / third-party action packages.
- USB/BLE HID mode (we are WiFi-first; host actions go through the agent).
