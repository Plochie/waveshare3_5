# Settings Page - Phase 1 (shell + persistence + brightness + auto-dim)

## Context

The device needs a Settings page. This is Phase 1 of a larger settings effort;
later phases (not designed here) will add: WiFi known-network management,
date & time / NTP, About/device info, and factory reset.

An EEZ Studio `settings` screen already exists but is an empty placeholder
(just a code-built back button, no real content). The on-device home screen
(`src/apps/eez_demo/eez_demo.cpp`) already has a "Settings" nav button wired
to `settings_create()`.

Currently the backlight (`lib/hal_display`, pin `GFX_BL` = GPIO6) is a hard
digital on/off via `hal_display_backlight(bool)`. There is no settings
persistence layer; the existing precedent for small persisted data is
`src/core/wifi_store.{h,cpp}`, which reads/writes a simple line-based text
file (`/wifi_known.txt`) via `sdcard::` helpers.

## Scope (Phase 1)

- Settings screen shell with a header matching the logs/file_explorer/wifi
  header pattern (back button top-left, centered title, content from y=32).
- Screen brightness control (slider, 10-100%).
- Auto-dim: turn the backlight fully off after N seconds of touch
  inactivity, restore on next touch.
- Persistence of both settings to SD card, surviving reboot.

Out of scope (future phases): WiFi known-network manager, date & time, About
page, factory reset.

## Design

### 1. Persistence - `src/core/settings_store.{h,cpp}`

Same line-based key=value pattern as `wifi_store`. File: `/settings.txt`.

```cpp
namespace settings_store {
  struct settings_t {
    uint8_t brightness;    // 10-100 (%)
    uint16_t dim_timeout;  // seconds, 0 = never dim
  };

  // Returns defaults (brightness=100, dim_timeout=30) if the file is
  // missing or no SD card is mounted.
  settings_t load();

  // No-op if no SD card is mounted (same graceful degradation as wifi_store).
  void save(const settings_t &s);
}
```

File format (one `key=value` per line, mirrors `wifi_known.txt` style but
with named keys since there's no fixed record shape):
```
brightness=100
dim_timeout=30
```

### 2. HAL - PWM backlight (`lib/hal_display`)

Replace the digital on/off with `ledc` PWM (arduino-esp32 3.x API):

```cpp
// Configures GFX_BL as a PWM (ledc) output. Call once at boot.
void hal_display_backlight_init();

// Sets backlight duty cycle, 0-100 (%). 0 = fully off.
void hal_display_set_brightness(uint8_t pct);
```

`hal_display_backlight(bool on)` is removed (no longer used anywhere -
`main.cpp` is its only caller and is updated below).

Implementation: `ledcAttach(GFX_BL, 5000 /* Hz */, 8 /* bit resolution */)` in
`hal_display_backlight_init()`; `hal_display_set_brightness(pct)` maps
0-100 -> duty 0-255 via `ledcWrite(GFX_BL, pct * 255 / 100)`.

### 3. Runtime brightness & auto-dim - `src/core/display_power.{h,cpp}`

Owns live brightness/dim state; the settings UI and `main.cpp` go through
this rather than touching `settings_store`/HAL directly.

```cpp
namespace display_power {
  // Loads settings_store, applies brightness via HAL, starts the auto-dim
  // timer. Call once from main.cpp after hal_display_backlight_init().
  void init();

  // Applies + persists immediately (settings_store::save()).
  void set_brightness(uint8_t pct);   // clamped 10-100
  void set_dim_timeout(uint16_t sec); // one of 0,15,30,60,120,300

  uint8_t brightness();
  uint16_t dim_timeout();
}
```

- `init()` loads saved settings, calls `hal_display_set_brightness(brightness)`,
  and starts an `lv_timer` polling every ~1s.
- The timer compares `lv_display_get_inactive_time(disp)` (ms since last
  touch, tracked automatically by LVGL) against `dim_timeout * 1000`. If
  exceeded and the backlight isn't already off -> `hal_display_set_brightness(0)`.
  On the next touch, LVGL's inactive time resets to 0, so the timer detects
  "no longer idle" and restores `hal_display_set_brightness(brightness())`.
- `dim_timeout == 0` ("Never") disables the dimming check entirely - the
  timer still runs but never turns the backlight off.
- Setters write through to `settings_store::save()` immediately (the file is
  tiny and writes are infrequent - no debouncing).

### 4. Settings screen UI

**Header** - restructure the existing EEZ `settings` screen to match the
logs/file_explorer/wifi header pattern:
- `settings_back_btn` - `LVGLLabelWidget`, text `" Back"`, at
  (8,8,70x16,content), color `accent_blue` (`#60A5FA`). Clickable ->
  `screen_manager::pop()`.
- `settings_title` - `LVGLLabelWidget`, text "SETTINGS", centered at
  (0,8,320x16,content), `text_align CENTER`, color `text_secondary` (`#999999`).

**Content** (starts at y=32), two control groups, no card backgrounds (minimal
v1 list):

- **Brightness**
  - `brightness_label` - static "Brightness" label, left-aligned.
  - `brightness_value` - "100%" label, right-aligned on the same row.
  - `brightness_slider` - `LVGLSliderWidget`, range 10-100, full width
    (288px), positioned below the label row.
  - `VALUE_CHANGED` handler: `display_power::set_brightness(value)` and
    update `brightness_value` text to `"<value>%"`.

- **Auto-dim**
  - `dim_label` - static "Screen off after" label, left-aligned.
  - `dim_value_btn` - clickable label showing the current value ("Off",
    "15s", "30s", "1m", "2m", "5m"), right-aligned on the same row as
    `dim_label`.
  - `CLICKED` handler: cycles through `{0, 15, 30, 60, 120, 300}` seconds (in
    order, wrapping), calls `display_power::set_dim_timeout(...)`, and
    updates `dim_value_btn` text via a small seconds->label helper.

On `settings_create()`: initialize `brightness_slider` value and both
`brightness_value`/`dim_value_btn` labels from
`display_power::brightness()` / `display_power::dim_timeout()`.

### 5. Wiring summary

- `src/core/settings_store.{h,cpp}` - new, as above.
- `src/core/display_power.{h,cpp}` - new, as above.
- `lib/hal_display`:
  - Add `hal_display_backlight_init()` and `hal_display_set_brightness(uint8_t)`.
  - Remove `hal_display_backlight(bool)`.
- `src/main.cpp`:
  - Replace `hal_display_backlight(true)` with
    `hal_display_backlight_init();` followed by `display_power::init();`
    (after `lv_init()`/display setup, since `display_power::init()` needs
    `disp` for `lv_display_get_inactive_time`).
- `src/apps/settings/settings.cpp` - rewritten to use the new EEZ objects
  (`settings_back_btn`, `settings_title`, `brightness_label`,
  `brightness_value`, `brightness_slider`, `dim_label`, `dim_value_btn`) and
  wire the handlers described above.
- EEZ project (`eez-studio/eez-studio3-5.eez-project`) - `settings` screen
  restructured per section 4, then re-exported by the user (regenerates
  `src/ui/eez_export/screens.{h,c}`).

## Verification

- `~/.platformio/penv/bin/pio run` builds clean.
- Flash + monitor:
  - Settings screen shows header (Back / "SETTINGS"), brightness slider +
    value, and auto-dim row.
  - Dragging the brightness slider changes screen brightness live and
    updates the "%" label.
  - Tapping the auto-dim row cycles Off -> 15s -> 30s -> 1m -> 2m -> 5m -> Off
    and updates its label.
  - Reboot the device: brightness and auto-dim settings persist (read back
    from `/settings.txt`).
  - With a short auto-dim timeout set, leave the device untouched - screen
    goes dark after the timeout; a touch anywhere restores the previous
    brightness.
  - Back button returns to the home screen.
