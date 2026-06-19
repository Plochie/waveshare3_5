# Serial Monitor — Screen Design

## Purpose

A new app screen (`app_category::debug`, accent amber) that monitors and
talks to an external device over a **hardware UART** (`Serial1` on two free
ESP32-S3 GPIO pins). It renders incoming bytes as a scrolling, color-coded
terminal log, lets the user send lines via an on-screen keyboard or
quick-send macro buttons, and exposes tappable config cyclers (baud,
framing, line-ending) plus RX/TX activity indicators.

This spec covers the **EEZ Studio screen layout** (new `userPages` entry
`serial_monitor`). Export regenerates `src/ui/eez_export/`; the screen is
wired up in `src/apps/serial_monitor/` and registered in `app_registry.cpp`
(and added to the `eez_demo` home nav) as a follow-up. The actual RX/TX GPIO
pins are chosen at wiring time.

## Layout (320x480)

### Header (top = 8, matches wifi_manager/settings/i2c_spi_scanner)
- `serial_back_btn` — LVGLLabelWidget, " Back" (LV_SYMBOL_LEFT), left=8
  width=70, text_color accent_blue (#60A5FA). CLICKABLE + pop in wrapper.
- `serial_title` — LVGLLabelWidget, "SERIAL MONITOR", left=0 width=320,
  text_align CENTER, text_color text_secondary (#999999).
- No voltage readout (the diagram's "3.7V" is dropped — no voltage sense,
  consistent with the I2C scanner).

### Config bar (top = 32, all elements tappable)
- `baud_btn` — LVGLLabelWidget "115200", left=8. Cycles common baud rates
  (9600/19200/38400/57600/115200/230400) on tap; re-inits Serial1.
- `framing_btn` — LVGLLabelWidget "8N1", positioned right of baud. Cycles
  framing (8N1/8E1/8O1/7E1) on tap.
- `rx_dot` / `tx_dot` — small LVGLContainerWidget circles (built as shapes,
  NOT a "●" glyph, to avoid font tofu), each followed by a `rx_label`/
  `tx_label` ("RX"/"TX", text_muted). Dots flash accent_green/amber briefly
  on RX/TX traffic, idle = bg_surface.
- `lineend_btn` — LVGLLabelWidget "CR+LF", right-aligned (left≈250). Cycles
  CR+LF → CR → LF → (none) on tap; sets the bytes appended to sent lines.

### Terminal output (top = 56, ~300px tall)
- `term_output` — LVGLContainerWidget, left=8 top=56 width=304 height≈300,
  bg_card (#111115), radius 8, border #2A2A2E. Turned into a scrolling
  flex-column in the wrapper; each received line is its own wrapped label,
  color-coded by a simple severity heuristic on the text:
  - contains "ERR"/"FAIL" → accent_red
  - contains "WARN" → accent_amber
  - contains "OK"/"ready"/"connected" → accent_green
  - prompt lines starting ">" → accent_blue
  - otherwise → text_primary / text_muted
  EEZ holds 2-3 mockup lines for design reference; the real buffer is built
  in C (reuses the Logs app's per-line label pattern, bounded to the most
  recent N lines).

### Input row (top ≈ 364)
- `term_input` — LVGLTextareaWidget, left=8 width≈230 height=36, oneLineMode
  true, placeholder "type a command…", bg_card/border styling (mirrors
  wifi_connect's `pw_input`, minus passwordMode).
- `send_btn` — LVGLLabelWidget " Send" (or "Send"), right side
  (left≈250 width=60), accent_green, CLICKABLE. Sends the textarea contents
  + current line-ending, echoes locally, clears the field.

### Macro row (top ≈ 408)
- Five small tappable LVGLLabelWidget "buttons", evenly spaced:
  `macro_at_btn` "AT", `macro_rst_btn` "RST", `macro_help_btn` "help",
  `macro_clr_btn` "clr", `macro_add_btn` "+ macro".
  - AT/RST/help send fixed strings (`AT`, `AT+RST`, `help`) + line-ending.
  - clr clears `term_output`.
  - "+ macro" is a placeholder (no-op for now; future: user-defined macros).

### On-screen keyboard (hidden by default)
- `serial_keyboard` — LVGLKeyboardWidget, left=0 top=353 width=320
  height=127 (matches wifi_connect's `pw_keyboard`). Hidden
  (LV_OBJ_FLAG_HIDDEN) until `term_input` gains focus; shown on
  LV_EVENT_FOCUSED, hidden on LV_EVENT_DEFOCUSED/READY. Routed to the
  textarea via `lv_keyboard_set_textarea`. Its READY (checkmark) also sends
  the line. While visible it overlays the terminal/macro rows.
- Because the keyboard (top=353) would cover the default input row, the
  wrapper temporarily raises `term_input` + `send_btn` to just above the
  keyboard (y≈325) on FOCUSED and restores their bottom positions on
  DEFOCUSED/READY, so the user always sees what they're typing. (Pure
  wrapper logic — EEZ keeps the diagram's bottom positions as defaults.)

## Wiring notes (follow-up, not this spec)
- New `src/apps/serial_monitor/serial_monitor.{h,cpp}` with
  `lv_obj_t *serial_monitor_create()`; apply `styles::style_screen`.
- `Serial1.begin(baud, config, RX_PIN, TX_PIN)`; poll in an `lv_timer`,
  append bytes to a line buffer, rebuild `term_output` on change.
- Register in `app_registry.cpp` (debug category) + add a nav button in
  `eez_demo.cpp`.
- All identifiers are globally unique (prefixed) to avoid the flat
  `objects_t` collision we hit with `scan_btn`.

## Out of scope (future work)
- User-defined/editable macros ("+ macro" is a placeholder).
- Saving terminal output to SD or scrollback search.
- Auto-baud detection; hardware flow control (RTS/CTS).
- Hex/binary view mode (text/ASCII only for now).
