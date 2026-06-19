# WLED Controller — Multi-Screen App Design

## Purpose

A device app (`app_category::network`, accent blue) that discovers and
controls [WLED](https://kno.wled.ge/) devices on the local WiFi network over
the WLED JSON HTTP API. Users see a list of saved WLED devices, quick-toggle
or dim them inline, open a per-device control screen (color via hue +
saturation sliders, brightness, quick-color palette), and add new devices by
mDNS auto-discovery or manual IP/hostname entry.

The app is built across **4 EEZ screens** + **2 core modules**, delivered in
three wiring phases. All screens are designed in EEZ first and exported in one
pass; wiring proceeds phase by phase.

## Build sequencing (wiring phases)

- **Phase 1 — Foundation + list + manual add:** `wled_store`, `wled_client`,
  `wled_lights` (list + inline toggle/brightness), `wled_add_manual`.
  Outcome: add a device by IP, see it, toggle/dim it. Usable end-to-end.
- **Phase 2 — Device detail:** `wled_detail` (hue/sat/brightness sliders,
  quick colors, power).
- **Phase 3 — mDNS discovery:** `wled_add` scan/results/empty states.

## Core modules (wired Phase 1)

### `core/wled_store` (SD persistence, modeled on `core/wifi_store`)
- Persists the saved device list to `/wled.txt` on the SD card, line-based.
- `struct device { String name; String host; uint16_t leds; };`
- `size_t load(device out[], size_t max)` — most-recent first.
- `void save(const char *host, const char *name)` — add/update by host,
  persist immediately; no-op if SD not mounted.
- `void remove(const char *host)` — forget a device.
- `MAX_DEVICES = 16`.

### `core/wled_client` (WLED JSON HTTP API)
- Uses Arduino-ESP32 `HTTPClient` over the active STA connection.
- `bool get_status(const char *host, status &out)` — `GET http://<host>/json`
  → parse `state` (on, bri, seg[0].col[0]) + `info` (name, leds.count, ver).
- `bool set_power(const char *host, bool on)` — `POST /json/state {"on":...}`.
- `bool set_brightness(const char *host, uint8_t bri)` — `{"bri":0-255}`.
- `bool set_color(const char *host, uint8_t r,g,b)` —
  `{"seg":[{"col":[[r,g,b]]}]}`.
- `bool probe(const char *host, info &out)` — used by manual-add Test
  Connection (validates it's a WLED device, returns name/leds/ver/latency).
- Short timeouts (~1s); all calls return false on failure (no exceptions).
- JSON parsing via ArduinoJson (add to `platformio.ini` lib_deps if absent).

## Screen layouts (320x480, header matches existing pages)

### ① `wled_lights` — device list (app root)
- Header (top=8): `wled_back_btn` (" Back", accent_blue) · `wled_title`
  ("WLED LIGHTS", centered, text_secondary) · `wled_add_btn` ("+", right,
  accent_green, top=8) → pushes `wled_add`.
- `all_lights_card` (static container, top=32): `all_lights_name`
  ("All Lights"), `all_lights_summary` ("3 of 4 on · avg 62%"). Master
  power `lv_switch` added in the wrapper (right side). Toggling sets power on
  all devices.
- `devices_label` ("DEVICES · N", top≈84, text_secondary).
- `device_list` (container, top≈104, fills remaining height): emptied + built
  in C from `wled_store` (the `wifi_manager` pattern). Each card (built in C):
  color swatch circle (live device color), name, "host · N LEDs",
  brightness bar, power `lv_switch`. Tapping a card pushes `wled_detail`;
  the switch quick-toggles power. Off devices render dimmed.
- `wled_hint` (footer, text_muted): "tap device for controls · + to discover".
- EEZ holds 1-2 mockup cards for design reference (never shown at runtime).

### ② `wled_detail` — device control
- Header (top=8): `detail_back_btn` (" Back") · `detail_title` (device
  name, set in wrapper) · power `lv_switch` added in C at header right.
- `detail_subtitle` ("host · N LEDs · online", top=32, text_secondary).
- `live_preview` (container, top≈52, ~24px): a bar that reflects the current
  RGB color (set in wrapper). Decorative.
- Color picker (hue + saturation sliders; no LVGL color wheel in v9.2.2):
  - `hue_label` ("HUE") + `hue_slider` (min 0 max 359). Rainbow gradient
    background applied in the wrapper.
  - `sat_label` ("SATURATION") + `sat_slider` (min 0 max 255).
- `bri_label` ("BRIGHTNESS · 75%", value updated in wrapper) + `bri_slider`
  (min 0 max 255).
- `quick_label` ("QUICK COLORS") + `quick_colors` (container, built in C):
  preset swatches (warm white, white, red, green, blue, purple, pink) + a
  "+" placeholder. Tapping a swatch sets color.
- `solid_label` ("Solid color mode") + `solid_sub` ("effects/presets —
  coming in a future update") + a `lv_switch` (C). **Placeholder** — effects
  deferred; toggle is inert for now.
- `detail_hint` (footer): "drag sliders to adjust · encoder → brightness".
- Slider drags debounce-POST to `wled_client` (~150ms) to avoid flooding.

### ③ `wled_add` — discovery flow (one screen, 3 toggled sections)
- Header (top=8): `add_back_btn` (" Back") · `add_title` ("ADD WLED
  DEVICE", centered).
- `scan_section` (container): a `lv_spinner` (built in C), `scan_status`
  ("Scanning network…"), `scan_sub` ("Looking for WLED devices via mDNS"),
  `scan_timer` ("3.2s / 5s", updated in C), `scan_skip_btn` ("Skip — add
  manually instead") → pushes `wled_add_manual`.
- `results_section` (container): `results_banner` ("Found N devices on
  network", green) + `results_list` (container, built in C — each row:
  name, host·LEDs·FW, an add `lv_switch`; already-saved devices shown
  dimmed/"ALREADY ADDED").
- `empty_section` (container): `empty_title` ("No WLED devices found"),
  `empty_sub` ("Scan completed · 0 devices responded"), `empty_tips` (the
  "TRY THIS" hints). A red "✗" mark built in C.
- Shared footer row (hidden while scanning): `scan_again_btn` ("Scan again")
  → restarts scan; `add_manual_btn` ("+ Add manually") → pushes
  `wled_add_manual`.
- Wrapper shows exactly one of scan/results/empty; mDNS query (Phase 3) via
  `ESPmDNS` browsing `_http._tcp` / WLED hostnames, auto-cancel after 5s.
- A successful add (here or in manual) returns to `wled_lights`.

### ④ `wled_add_manual` — manual entry (keyboard fixed bottom, `wifi_connect` pattern)
- Header (top=8): `manual_back_btn` (" Back") · `manual_title` ("ADD
  MANUALLY", centered).
- `manual_input_label` ("IP ADDRESS OR HOSTNAME", top=32, text_secondary).
- `manual_input` (LVGLTextareaWidget, top≈52, oneLine, placeholder
  "192.168.1.x").
- `manual_test_btn` ("Test Connection", top≈96) → calls
  `wled_client::probe`.
- `manual_result` (container, hidden until tested): `manual_result_name` +
  `manual_result_info` ("N LEDs · FW x · host · responded in Nms").
  Green on success, red on failure.
- `manual_add_btn` ("Add Device", enabled only after a successful probe) →
  `wled_store::save` then pop back to `wled_lights`.
- `manual_keyboard` (LVGLKeyboardWidget, **NUMBER mode**, fixed bottom
  0/353/320/127). Routed to `manual_input`. Matches the numeric-keypad
  mockup; full hostname typing (letters) is deferred.

## Conventions / constraints
- All widget `identifier`s are globally unique (prefixed per screen) because
  the exported `objects_t` is one flat struct (the `scan_btn` collision we
  hit earlier).
- Switches (`lv_switch`), spinners (`lv_spinner`), dynamic list cards, and
  quick-color swatches are built in C (LVGL has no EEZ switch/spinner widget
  used here); EEZ provides static chrome + mockups only.
- Apply `styles::style_screen` to each screen root in its wrapper (EEZ
  `darkTheme` doesn't set a bg).
- Symbols use `LV_SYMBOL_*` only (font subset has no arbitrary glyphs);
  custom marks (swatch circles, ✗, brightness bars) are composed from shapes.

## Out of scope (future work)
- WLED effects, presets, palettes, segments (the "Solid color mode" toggle is
  a placeholder).
- Per-device rename/reorder; multi-segment control.
- Saturation-aware color sync back from device (we push color; read-back is
  best-effort for the swatch).
- Full-keyboard hostname entry (NUMBER keypad / IP only for now).
- Websocket live updates (we poll on screen open / after writes).
