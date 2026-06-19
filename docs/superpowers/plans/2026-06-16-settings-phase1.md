# Settings Page Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a working Settings screen with persisted screen-brightness control (PWM) and an auto-dim-after-idle timeout, per `docs/superpowers/specs/2026-06-16-settings-phase1-design.md`.

**Architecture:** A new `settings_store` module persists a small key=value file (`/settings.txt`) on the SD card. `hal_display` gains PWM (`ledc`) brightness control replacing the old digital on/off. A new `display_power` module owns live brightness/auto-dim state, applying it via `hal_display` and persisting via `settings_store`. The EEZ `Settings` screen gets a header (matching logs/wifi/files) plus a brightness slider and an auto-dim cycle button; `settings.cpp` wires these to `display_power`.

**Tech Stack:** PlatformIO/Arduino (ESP32-S3), LVGL v9.2.2, EEZ Studio (LVGL export), arduino-esp32 `ledc` PWM API.

There is no test framework in this project (per CLAUDE.md) — "tests" below means `~/.platformio/penv/bin/pio run` build verification plus on-device manual checks in the final task.

**Note:** This project directory is not a git repository, so skip all `git add`/`git commit` steps below — they're listed for reference but have no effect here.

---

### Task 1: `settings_store` persistence module

**Files:**
- Create: `src/core/settings_store.h`
- Create: `src/core/settings_store.cpp`

- [ ] **Step 1: Create the header**

`src/core/settings_store.h`:
```cpp
#pragma once

#include <stdint.h>

// Small persisted key=value settings file on the SD card (/settings.txt),
// following the same line-based convention as core/wifi_store.
namespace settings_store {

struct settings_t {
  uint8_t brightness;    // 10-100 (%)
  uint16_t dim_timeout;  // seconds, 0 = never dim
};

// Returns defaults (brightness=100, dim_timeout=30) if /settings.txt is
// missing or no SD card is mounted.
settings_t load();

// No-op if no SD card is mounted.
void save(const settings_t &s);

} // namespace settings_store
```

- [ ] **Step 2: Create the implementation**

`src/core/settings_store.cpp`:
```cpp
#include "core/settings_store.h"

#include <Arduino.h>

#include "hal_sdcard.h"

namespace settings_store {

static constexpr const char *PATH = "/settings.txt";
static constexpr settings_t DEFAULTS = {100, 30};

settings_t load()
{
  settings_t s = DEFAULTS;
  if (!sdcard::is_mounted()) {
    return s;
  }

  String content = sdcard::read_string(PATH);
  int pos = 0;
  while (pos < (int)content.length()) {
    int nl = content.indexOf('\n', pos);
    String line = (nl < 0) ? content.substring(pos) : content.substring(pos, nl);
    pos = (nl < 0) ? content.length() : nl + 1;

    int eq = line.indexOf('=');
    if (eq < 0) {
      continue;
    }
    String key = line.substring(0, eq);
    String value = line.substring(eq + 1);

    if (key == "brightness") {
      int v = value.toInt();
      if (v >= 10 && v <= 100) {
        s.brightness = (uint8_t)v;
      }
    } else if (key == "dim_timeout") {
      int v = value.toInt();
      if (v >= 0 && v <= 65535) {
        s.dim_timeout = (uint16_t)v;
      }
    }
  }
  return s;
}

void save(const settings_t &s)
{
  if (!sdcard::is_mounted()) {
    return;
  }

  String content;
  content += "brightness=";
  content += s.brightness;
  content += '\n';
  content += "dim_timeout=";
  content += s.dim_timeout;
  content += '\n';
  sdcard::write(PATH, content.c_str());
}

} // namespace settings_store
```

- [ ] **Step 3: Build to verify it compiles**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]` (this module isn't called from anywhere yet, but LDF's `deep+`
mode only compiles included files — it won't be pulled in until Task 4
includes it. This step just confirms there's no syntax error by temporarily
checking the file compiles in isolation is unnecessary; skip straight to
Task 4 if you prefer. If you want an early check, proceed to Task 3 first,
since `display_power.cpp` includes this header.)

- [ ] **Step 4: Commit**

```bash
git add src/core/settings_store.h src/core/settings_store.cpp
git commit -m "Add settings_store persistence module"
```

---

### Task 2: PWM backlight in `hal_display`

**Files:**
- Modify: `lib/hal_display/src/hal_display.h`
- Modify: `lib/hal_display/src/hal_display.cpp`

- [ ] **Step 1: Update the header**

In `lib/hal_display/src/hal_display.h`, replace:
```cpp
// Enables/disables the backlight (GFX_BL pin).
void hal_display_backlight(bool on);
```
with:
```cpp
// Configures the backlight pin (GFX_BL) for PWM brightness control via ledc.
// Call once at boot, after hal_display_init().
void hal_display_backlight_init();

// Sets backlight brightness, 0-100 (%). 0 = fully off (no light).
void hal_display_set_brightness(uint8_t pct);
```

- [ ] **Step 2: Update the implementation**

In `lib/hal_display/src/hal_display.cpp`, replace:
```cpp
void hal_display_backlight(bool on)
{
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, on ? HIGH : LOW);
}
```
with:
```cpp
void hal_display_backlight_init()
{
  ledcAttach(GFX_BL, 5000 /* Hz */, 8 /* bit resolution */);
  ledcWrite(GFX_BL, 255);
}

void hal_display_set_brightness(uint8_t pct)
{
  if (pct > 100) {
    pct = 100;
  }
  ledcWrite(GFX_BL, (uint32_t)pct * 255 / 100);
}
```

- [ ] **Step 3: Build to verify (will fail until Task 4 updates the caller)**

Run: `~/.platformio/penv/bin/pio run`
Expected: FAIL — `src/main.cpp` still calls the removed
`hal_display_backlight(true)`, producing an "undeclared" / "no matching
function" error. This confirms the old symbol is gone; Task 4 fixes the call
site.

- [ ] **Step 4: Commit**

```bash
git add lib/hal_display/src/hal_display.h lib/hal_display/src/hal_display.cpp
git commit -m "Replace digital backlight on/off with PWM brightness control"
```

---

### Task 3: `display_power` runtime module

**Files:**
- Create: `src/core/display_power.h`
- Create: `src/core/display_power.cpp`

- [ ] **Step 1: Create the header**

`src/core/display_power.h`:
```cpp
#pragma once

#include <stdint.h>

// Owns live screen brightness and auto-dim state. The settings UI and
// main.cpp go through this rather than touching settings_store/hal_display
// directly, so brightness/dim-timeout changes are applied and persisted in
// one place.
namespace display_power {

// Loads persisted settings, applies brightness via hal_display, and starts
// the auto-dim idle timer. Call once after the LVGL display is created.
void init();

// Clamped to 10-100. Applies immediately (unless currently dimmed) and
// persists via settings_store.
void set_brightness(uint8_t pct);

// One of 0 (never dim), 15, 30, 60, 120, 300 seconds. Persists via
// settings_store.
void set_dim_timeout(uint16_t sec);

uint8_t brightness();
uint16_t dim_timeout();

} // namespace display_power
```

- [ ] **Step 2: Create the implementation**

`src/core/display_power.cpp`:
```cpp
#include "core/display_power.h"

#include <lvgl.h>

#include "core/settings_store.h"
#include "hal_display.h"

namespace display_power {

static settings_store::settings_t s_settings;
static bool s_dimmed = false;

// Polls LVGL's per-display idle time once a second. Backlight goes fully off
// after dim_timeout seconds of no touch, and is restored to the saved
// brightness on the next touch (LVGL resets inactive time automatically).
static void check_idle_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  lv_display_t *disp = lv_display_get_default();
  uint32_t idle_ms = lv_display_get_inactive_time(disp);

  if (s_settings.dim_timeout != 0 &&
      idle_ms >= (uint32_t)s_settings.dim_timeout * 1000) {
    if (!s_dimmed) {
      hal_display_set_brightness(0);
      s_dimmed = true;
    }
  } else if (s_dimmed) {
    hal_display_set_brightness(s_settings.brightness);
    s_dimmed = false;
  }
}

void init()
{
  s_settings = settings_store::load();
  hal_display_set_brightness(s_settings.brightness);
  lv_timer_create(check_idle_cb, 1000, NULL);
}

void set_brightness(uint8_t pct)
{
  if (pct < 10) {
    pct = 10;
  } else if (pct > 100) {
    pct = 100;
  }
  s_settings.brightness = pct;
  if (!s_dimmed) {
    hal_display_set_brightness(pct);
  }
  settings_store::save(s_settings);
}

void set_dim_timeout(uint16_t sec)
{
  s_settings.dim_timeout = sec;
  settings_store::save(s_settings);
}

uint8_t brightness()
{
  return s_settings.brightness;
}

uint16_t dim_timeout()
{
  return s_settings.dim_timeout;
}

} // namespace display_power
```

- [ ] **Step 3: Commit**

```bash
git add src/core/display_power.h src/core/display_power.cpp
git commit -m "Add display_power module for brightness and auto-dim"
```

---

### Task 4: Wire `display_power` into `main.cpp`

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add includes**

In `src/main.cpp`, add to the includes near the other `core/` includes (after
`#include "core/wifi_autoconnect.h"` at line 18):
```cpp
#include "core/display_power.h"
```

- [ ] **Step 2: Replace the old backlight call**

Replace (around line 122):
```cpp
  gfx = hal_display_init();
  hal_display_backlight(true);
  if (!gfx) {
```
with:
```cpp
  gfx = hal_display_init();
  hal_display_backlight_init();
  if (!gfx) {
```

- [ ] **Step 3: Initialize display_power after the display is created**

In the `else` block where `disp = lv_display_create(...)` and the touch
indev are set up (around lines 162-170), add `display_power::init()` after
`screen_manager::init()`:
```cpp
    styles::init();
    screen_manager::init();
    display_power::init();
    screen_manager::push(eez_demo_create());
    wifi_autoconnect::start();
```

- [ ] **Step 4: Build to verify**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]`

- [ ] **Step 5: Commit**

```bash
git add src/main.cpp
git commit -m "Initialize display_power at boot"
```

---

### Task 5: EEZ Studio — Settings screen header + brightness/dim controls

**Files:**
- Modify: `eez-studio/eez-studio3-5.eez-project` (via temporary script, deleted after success)
- Create then delete: `_tmp_add_settings_phase1.py`

This task edits the EEZ project JSON to add the new widgets to the
`Settings` page (which already exists as an empty placeholder — its
`objects.settings` screen currently has zero children). After this script
runs successfully, **stop and ask the user to open EEZ Studio and re-export**
before continuing to Task 6 — `src/ui/eez_export/screens.h` must contain the
new identifiers (`settings_back_btn`, `settings_title`, `brightness_label`,
`brightness_value`, `brightness_slider`, `dim_label`, `dim_value_btn`) before
Task 6's code will compile.

- [ ] **Step 1: Write the script**

`_tmp_add_settings_phase1.py` (in the project root):
```python
import json
import uuid

PATH = "eez-studio/eez-studio3-5.eez-project"

with open(PATH) as f:
    proj = json.load(f)


def label(identifier, text, left, top, width, height, height_unit,
          text_color, text_align=None):
    style_def = {"text_color": text_color}
    if text_align:
        style_def["text_align"] = text_align
    return {
        "objID": str(uuid.uuid4()),
        "type": "LVGLLabelWidget",
        "left": left,
        "top": top,
        "width": width,
        "height": height,
        "customInputs": [],
        "customOutputs": [],
        "style": {
            "objID": str(uuid.uuid4()),
            "useStyle": "default",
            "conditionalStyles": [],
            "childStyles": []
        },
        "timeline": [],
        "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px",
        "topUnit": "px",
        "widthUnit": "px",
        "heightUnit": height_unit,
        "children": [],
        "widgetFlags": "CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal",
        "clickableFlagType": "literal",
        "flagScrollbarMode": "",
        "flagScrollDirection": "",
        "scrollSnapX": "",
        "scrollSnapY": "",
        "checkedStateType": "literal",
        "disabledStateType": "literal",
        "states": "",
        "localStyles": {
            "objID": str(uuid.uuid4()),
            "definition": {"MAIN": {"DEFAULT": style_def}}
        },
        "group": "",
        "groupIndex": 0,
        "text": text,
        "textType": "literal",
        "longMode": "WRAP",
        "recolor": False
    }


def slider(identifier, left, top, width, height, min_v, max_v, value):
    return {
        "objID": str(uuid.uuid4()),
        "type": "LVGLSliderWidget",
        "left": left,
        "top": top,
        "width": width,
        "height": height,
        "customInputs": [],
        "customOutputs": [],
        "style": {
            "objID": str(uuid.uuid4()),
            "useStyle": "default",
            "conditionalStyles": [],
            "childStyles": []
        },
        "timeline": [],
        "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px",
        "topUnit": "px",
        "widthUnit": "px",
        "heightUnit": "px",
        "children": [],
        "widgetFlags": "CLICKABLE|CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_ON_FOCUS|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal",
        "clickableFlag": True,
        "clickableFlagType": "literal",
        "checkedStateType": "literal",
        "disabledStateType": "literal",
        "states": "",
        "localStyles": {"objID": str(uuid.uuid4())},
        "group": "",
        "groupIndex": 0,
        "min": min_v,
        "minType": "literal",
        "max": max_v,
        "maxType": "literal",
        "mode": "NORMAL",
        "value": value,
        "valueType": "literal",
        "previewValue": value,
        "valueLeft": 0,
        "valueLeftType": "literal",
        "previewValueLeft": 0,
        "enableAnimation": False
    }


settings_page = None
for page in proj["userPages"]:
    if page["name"] == "Settings":
        settings_page = page
        break

if settings_page is None:
    raise SystemExit("Settings page not found")

children = settings_page["components"][0]["children"]

children.append(label("settings_back_btn", " Back", 8, 8, 70, 16,
                       "content", "#60A5FA"))
children.append(label("settings_title", "SETTINGS", 0, 8, 320, 16,
                       "content", "#999999", text_align="CENTER"))

children.append(label("brightness_label", "Brightness", 16, 40, 150, 16,
                       "content", "#F0F0F0"))
children.append(label("brightness_value", "100%", 154, 40, 150, 16,
                       "content", "#999999", text_align="RIGHT"))
children.append(slider("brightness_slider", 16, 64, 288, 10, 10, 100, 100))

children.append(label("dim_label", "Screen off after", 16, 100, 150, 16,
                       "content", "#F0F0F0"))
children.append(label("dim_value_btn", "30s", 154, 100, 150, 16,
                       "content", "#60A5FA", text_align="RIGHT"))

with open(PATH, "w") as f:
    json.dump(proj, f, indent=2)

print("Settings page children:",
      [c.get("identifier", c["type"]) for c in children])
```

- [ ] **Step 2: Run the script**

Run: `python3 _tmp_add_settings_phase1.py`
Expected output ends with:
```
Settings page children: ['settings_back_btn', 'settings_title', 'brightness_label', 'brightness_value', 'brightness_slider', 'dim_label', 'dim_value_btn']
```

- [ ] **Step 3: Delete the temporary script**

```bash
rm _tmp_add_settings_phase1.py
```

- [ ] **Step 4: Ask the user to re-export**

Tell the user: the EEZ Studio `Settings` page now has a header (Back /
"SETTINGS"), a Brightness label + value + slider, and a "Screen off after"
row with a cycling value button. Ask them to open EEZ Studio and re-export,
which regenerates `src/ui/eez_export/screens.{h,c}`. **Do not start Task 6
until the user confirms the re-export is done.**

- [ ] **Step 5: Verify the export (after user confirms)**

Run: `grep -o 'settings_back_btn\|settings_title\|brightness_label\|brightness_value\|brightness_slider\|dim_label\|dim_value_btn' src/ui/eez_export/screens.h | sort -u`
Expected: all 7 identifiers listed.

- [ ] **Step 6: Commit the EEZ project + regenerated export**

```bash
git add eez-studio/eez-studio3-5.eez-project src/ui/eez_export/screens.h src/ui/eez_export/screens.c
git commit -m "Add Settings screen header, brightness slider, and auto-dim control"
```

---

### Task 6: Rewrite `settings.cpp`

**Files:**
- Modify: `src/apps/settings/settings.cpp`

**Depends on:** Task 5 being re-exported (objects above must exist in
`src/ui/eez_export/screens.h`).

- [ ] **Step 1: Replace the file contents**

`src/apps/settings/settings.cpp`:
```cpp
#include "apps/settings/settings.h"

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "core/display_power.h"

static void back_event_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

// Brightness slider: live-applies + persists via display_power, and updates
// the "%" label.
static void brightness_changed_cb(lv_event_t *e)
{
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int32_t value = lv_slider_get_value(slider);
  display_power::set_brightness((uint8_t)value);
  lv_label_set_text_fmt(objects.brightness_value, "%d%%", (int)value);
}

// Cycles dim_value_btn through the preset auto-dim timeouts and updates the
// label + display_power state.
static const uint16_t kDimOptions[] = {0, 15, 30, 60, 120, 300};
static const char *kDimLabels[] = {"Off", "15s", "30s", "1m", "2m", "5m"};
static constexpr size_t kDimOptionCount = sizeof(kDimOptions) / sizeof(kDimOptions[0]);

static size_t dim_index_for(uint16_t seconds)
{
  for (size_t i = 0; i < kDimOptionCount; i++) {
    if (kDimOptions[i] == seconds) {
      return i;
    }
  }
  return 0;
}

static void dim_value_clicked_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  size_t next = (dim_index_for(display_power::dim_timeout()) + 1) % kDimOptionCount;
  display_power::set_dim_timeout(kDimOptions[next]);
  lv_label_set_text(objects.dim_value_btn, kDimLabels[next]);
}

lv_obj_t *settings_create()
{
  create_screen_settings();

  lv_obj_add_style(objects.settings, &styles::style_screen, 0);

  lv_obj_add_flag(objects.settings_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.settings_back_btn, back_event_cb, LV_EVENT_CLICKED, NULL);

  uint8_t brightness = display_power::brightness();
  lv_slider_set_value(objects.brightness_slider, brightness, LV_ANIM_OFF);
  lv_label_set_text_fmt(objects.brightness_value, "%d%%", (int)brightness);
  lv_obj_add_event_cb(objects.brightness_slider, brightness_changed_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_label_set_text(objects.dim_value_btn,
                     kDimLabels[dim_index_for(display_power::dim_timeout())]);
  lv_obj_add_flag(objects.dim_value_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.dim_value_btn, dim_value_clicked_cb, LV_EVENT_CLICKED, NULL);

  return objects.settings;
}
```

- [ ] **Step 2: Build to verify**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]`

- [ ] **Step 3: Commit**

```bash
git add src/apps/settings/settings.cpp
git commit -m "Wire Settings screen to display_power brightness and auto-dim controls"
```

---

### Task 7: On-device verification

**Files:** none (manual verification only)

- [ ] **Step 1: Flash and monitor**

Run: `~/.platformio/penv/bin/pio run -t upload -t monitor`

- [ ] **Step 2: Check the Settings screen**

From the home screen, tap "Settings". Verify:
- Header shows "‹ Back" (top-left) and "SETTINGS" (centered).
- "Brightness" row shows a label, a "100%" value, and a slider.
- "Screen off after" row shows a value button reading "30s".

- [ ] **Step 3: Check brightness slider**

Drag the brightness slider. Verify the screen visibly dims/brightens live and
the "%" label updates to match the slider position. Set it to roughly 50%.

- [ ] **Step 4: Check auto-dim cycling**

Tap the "Screen off after" value button repeatedly. Verify it cycles
Off -> 15s -> 30s -> 1m -> 2m -> 5m -> Off.

Set it to "15s".

- [ ] **Step 5: Check auto-dim behavior**

Leave the device untouched for ~15-20 seconds. Verify the screen goes
completely dark. Then tap anywhere on the screen — verify the backlight
returns to the ~50% brightness set in Step 3.

- [ ] **Step 6: Check persistence across reboot**

Reset the device (power cycle or reset button). After it boots back to the
home screen, open Settings again and verify the brightness slider still
shows ~50% and the auto-dim value button still shows "15s" (read back from
`/settings.txt`).

- [ ] **Step 7: Check Back navigation**

Tap "‹ Back" on the Settings screen. Verify it returns to the home screen.
