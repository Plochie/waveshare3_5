# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

PlatformIO/Arduino firmware for the **Waveshare ESP32-S3-Touch-LCD-3.5B**
(ESP32-S3R8, 16MB flash, 8MB Octal PSRAM). It's the foundation for a
multi-app "personal multi-tool" device: a grouped app launcher (320x480,
LVGL v9) hosting many small tools (instruments, network scanners, debug
tools, motion/IMU apps, camera apps, media/info apps).

## Hardware

- **Display:** AXS15231B, 320x480 RGB565 over QSPI, driven via Arduino_GFX
  (`Arduino_ESP32QSPI` + `Arduino_AXS15231B`)
- **Touch:** AXS15231B integrated touch over I2C @ 0x3B
  (`lib/esp_lcd_touch_axs15231b`)
- **LCD reset:** driven through a TCA9554 I2C IO-expander @ 0x20, pin 1 (not
  a direct GPIO) — handled in `lib/hal_display`
- Pins, I2C addresses, and screen dimensions are centralized in
  `include/board_config.h`

## Build / run commands

PlatformIO CLI is not on PATH — use the full path:
```
~/.platformio/penv/bin/pio run                       # build
~/.platformio/penv/bin/pio run -t upload              # build + flash
~/.platformio/penv/bin/pio run -t upload -t monitor   # flash + serial monitor
~/.platformio/penv/bin/pio device monitor             # serial monitor only (115200 baud)
~/.platformio/penv/bin/pio run -t clean               # clean build artifacts
```
There is a single environment: `waveshare_esp32s3_35`. No tests/linters are
configured.

## Architecture

### Hybrid module layout: drivers in `lib/`, app code in `src/`

- **`lib/`** — hardware drivers, one PlatformIO library per peripheral, each
  with a `src/` subfolder:
  - `esp_lcd_touch_axs15231b/` — touch driver (`bsp_touch_init/read/get_coordinates`)
  - `hal_display/` — `hal_display_init()` (TCA9554 reset sequence + panel
    `begin()`) and `hal_display_backlight(bool)`
- **`include/`** — `lv_conf.h` (LVGL config, resolved via
  `LV_CONF_INCLUDE_SIMPLE` + `-I include`) and `board_config.h` (pins/dims)
- **`src/`** — application code, organized by feature folder. `-I src` is
  set in `platformio.ini` so any file can use root-relative includes, e.g.
  `#include "ui/styles.h"`, `#include "apps/app_registry.h"`.
  - `main.cpp` — Arduino `setup()`/`loop()`: I2C/display/touch init via the
    HAL, LVGL init (PSRAM double frame buffers, `DIRECT_RENDER_MODE` full
    refresh), then hands off to `screen_manager` + `launcher`
  - `core/screen_manager.{h,cpp}` — push/pop navigation stack on top of
    `lv_scr_load`; `pop()` calls `lv_obj_delete()` on the screen being left
    (RAM discipline — don't just hide inactive screens)
  - `ui/styles.{h,cpp}` — the shared dark-theme color palette
    (`styles::bg_primary()`, `styles::accent_green()`, etc.) and shared
    `lv_style_t` objects (`styles::style_screen`, `styles::style_card`);
    call `styles::init()` once after `lv_init()`
  - `ui/launcher.{h,cpp}` — placeholder launcher screen; renders one button
    per `app_registry` entry, tapping pushes that app's screen via
    `screen_manager::push()`
  - `apps/app_common.h` — shared types: `app_category` enum (instruments,
    network, debug, motion, camera, media_info) and `app_descriptor_t`
    (`name`, `icon`, `category`, `create()` factory returning a new screen)
  - `apps/app_registry.{h,cpp}` — static table of all registered apps
    (`app_registry::all(count)`); **adding a new app means adding its
    descriptor to this table**, no dynamic discovery
  - `apps/<name>/<name>.{h,cpp}` — one folder per app; each exposes a single
    `lv_obj_t *<name>_create()` that builds and returns a new screen
    (`lv_obj_create(NULL)`), suitable for `screen_manager::push()`

### Adding a new app

1. Create `src/apps/<name>/<name>.{h,cpp}` with `lv_obj_t *<name>_create()`
2. Register it in `src/apps/app_registry.cpp`'s static table with a name,
   icon, and `app_category`
3. Use `ui/styles.h` colors/styles rather than hardcoded hex values, so the
   app matches its category's accent color

### EEZ Studio UI files (`eez-studio/`, `src/ui/eez_export/`)

`eez-studio/eez-studio3-5.eez-project` is an [EEZ Studio](https://www.envox.io/eezstudio/)
LVGL UI project (LVGL v9.2.2, 320x480) used to visually design screens.
Exporting from EEZ Studio writes generated C sources into
`src/ui/eez_export/`:

- `screens.h/.c` — `objects_t` struct (one `lv_obj_t*` field per named
  widget) + `create_screen_<name>()` / `tick_screen_<name>()` per screen,
  plus `ScreensEnum` (`SCREEN_ID_<NAME>`) and `create_screens()`
- `ui.h/.c` — `ui_init()` (calls `create_screens()`), `ui_tick()`, and
  `loadScreen(ScreensEnum)` (uses `lv_scr_load_anim`, fade-in)
- `styles.h/.c`, `fonts.h/.c`, `images.h/.c`, `actions.h`, `vars.h`,
  `structs.h` — supporting assets/styles/event-handler declarations,
  populated as the EEZ project grows

This is **regenerated on every export** — treat `eez_export/` as
generated/vendored code, don't hand-edit it (custom logic belongs in
`app_registry`/`screen_manager`/the relevant `apps/<name>` file, calling
into `create_screen_<name>()` / `objects.<name>` as needed). Currently the
project has a single placeholder screen (`main`, just a "Hello, world!"
label).

**Wired into the app registry** via `src/apps/eez_demo/eez_demo.{h,cpp}`
("EEZ Demo" entry, `app_category::instruments`): `eez_demo_create()` calls
`create_screen_main()` (from `ui/eez_export/screens.h`) and returns
`objects.main`. `create_screen_main()` builds a fresh `lv_obj_t` tree and
reassigns `objects.main` each time it's called, so it's safe to call
repeatedly — e.g. when `screen_manager::pop()` deletes the previous instance
and the launcher opens it again. **Pattern for wiring up new EEZ screens**:
add a `src/apps/<name>/` wrapper following `eez_demo` that calls the
corresponding `create_screen_<name>()` and returns `objects.<name>`, then
register it in `app_registry.cpp`.

### `demo_arduino/`

Waveshare's original Arduino example bundle (numbered `.ino` examples +
vendored libraries). Reference material only — not part of the PlatformIO
build. `src/main.cpp` is a port of example 10 (`10_lvgl_arduino_v9`).

### LVGL config

`include/lv_conf.h` sets `LV_COLOR_DEPTH 16`. LDF mode is `deep+`, so only
libraries actually `#include`d from `src/` get compiled.
