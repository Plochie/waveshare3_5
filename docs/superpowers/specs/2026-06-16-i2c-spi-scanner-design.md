# I2C / SPI Scanner — Screen Design

## Purpose

A new app screen (`app_category::instruments`, accent green) that scans the
I2C bus for responding devices, shows a friendly-name guess per address, and
renders an 8x16 address-map grid (0x00-0x7F) highlighting which addresses
responded. A mode toggle is shown for I2C/SPI, but only I2C scanning is
implemented for now — SPI is a placeholder tab for future work.

This design covers the **EEZ Studio screen layout** (`eez-studio/eez-studio3-5.eez-project`,
new `userPages` entry `i2c_spi_scanner`). The exported screen is wired up in
`src/apps/i2c_spi_scanner/` and registered in `app_registry.cpp` as a
follow-up step (out of scope for this spec, but the `objects.*` identifiers
below are chosen with that wiring in mind).

## Layout (320x480, matches wifi_manager/settings header convention)

### Header (top = 8, 16px tall, heightUnit "content")
- `scanner_back_btn` — LVGLLabelWidget, text " Back" (LV_SYMBOL_LEFT),
  left=8 width=70, text_color accent_blue (#60A5FA). Wrapper adds
  CLICKABLE flag + `screen_manager::pop()` on click.
- `scanner_title` — LVGLLabelWidget, text "I2C / SPI SCANNER", left=0
  width=320, text_align CENTER, text_color text_secondary (#999999).

### Mode toggle + Scan toolbar (top = 32, 24px tall)
- `mode_i2c_btn` — LVGLLabelWidget, text "I2C", left=8 width=50,
  heightUnit content, text_align CENTER. Active state styled with
  bg_color accent_green / text_color bg_primary (rounded pill look via
  radius). Wrapper toggles styling between this and `mode_spi_btn` on tap;
  default active = I2C.
- `mode_spi_btn` — LVGLLabelWidget, text "SPI", left=62 width=50, same
  sizing, default inactive styling (bg_card / text_secondary).
- `scan_btn` — LVGLLabelWidget, text " Scan" (LV_SYMBOL_REFRESH),
  left=250 width=60, text_align CENTER, text_color accent_green,
  CLICKABLE. Triggers an I2C bus scan in the wrapper.

### Found-devices card (top = 64)
- `found_count_label` — LVGLLabelWidget, text "FOUND — 0 DEVICES",
  left=8 width=304, heightUnit content, text_color text_secondary,
  letter-spacing-style caption (uppercase wording, matches reference image).
- `device_list` — LVGLContainerWidget, left=0 top=84 width=100% height=160px
  (px), bg_card styling, radius 8, border per styles::border(). Holds
  device row cards, scrollable if needed (flagScrollDirection VER).
  - 2-3 mockup rows for design reference (cleaned + rebuilt at runtime),
    each row ~36px tall:
    - address label (e.g. "0x3C"), left=12, width=60, monospace-ish,
      text_color accent_green
    - name label (e.g. "SSD1306"), left=80, width=160, text_color
      text_primary
    - type tag label (e.g. "OLED"), left=250, width=60, text_align RIGHT,
      text_color text_muted
  - Mockup rows: `(0x3C, SSD1306, OLED)`, `(0x68, MPU6050, IMU)`,
    `(0x76, BME280, Env)` — matches the reference image.

### Address map section (top ≈ 256, below device_list)
- `addr_map_title` — LVGLLabelWidget, text "ADDRESS MAP — 0X00 → 0X7F",
  left=8 width=304, heightUnit content, text_color text_secondary.
- `addr_grid` — LVGLContainerWidget, left=8 top below title, width=304
  height=120px (px units), bg_card styling, radius 8. **Empty in EEZ** —
  the wrapper builds a 16-column x 8-row grid of small cell containers
  (0x00-0x7F) programmatically (`lv_obj_create` in a loop), coloring each
  cell based on scan results (accent_green if device found, bg_surface
  otherwise). No identifiers needed for individual cells.
- `addr_hint` — LVGLLabelWidget, text "tap address for datasheet ·
  encoder scrolls list", left=8 width=304, text_align CENTER, text_color
  text_muted, small/caption styling. Positioned below `addr_grid`.

## Styling notes
- Apply `styles::style_screen` to the screen root (dark bg) in the wrapper,
  matching all other screens (EEZ `darkTheme` doesn't set screen bg).
- Card backgrounds use `styles::bg_card()` (#111115), borders
  `styles::border()` (#2A2A2E), radius 8 — consistent with wifi_manager
  network cards.
- Active/inactive toggle styling for `mode_i2c_btn`/`mode_spi_btn` is
  applied/swapped in the wrapper via `lv_obj_set_style_*` calls (EEZ
  localStyles just provide the default/inactive look).

## Out of scope (future work)
- SPI bus scanning (mode_spi_btn is a visual placeholder only).
- Voltage reading display (dropped per user decision).
- Tapping an address cell to show datasheet info.
- Actual `src/apps/i2c_spi_scanner/` wrapper + app_registry entry — this
  spec covers only the EEZ screen layout; wiring is a follow-up task once
  the user exports `screens.h`/`screens.c`.
