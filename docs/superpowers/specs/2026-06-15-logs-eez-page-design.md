# Logs app — EEZ Studio page design

**Date:** 2026-06-15
**Status:** Approved (design), pending implementation

## Goal

The "Logs" app currently exists only as hand-built LVGL code in
[`src/apps/logs/logs.cpp`](../../../src/apps/logs/logs.cpp). Every other recent
screen is designed in EEZ Studio and wired up in a thin `src/apps/<name>/`
wrapper. Bring Logs in line: add a `logs` page to
`eez-studio/eez-studio3-5.eez-project` so the **UI lives in the EEZ file** and
the code only does **wiring** (event handlers + dynamic content).

Guiding principle (user): *UI is always present in the EEZ file; wiring happens
in the C wrapper.* That's why the full header (Back / title / Clear) goes in
EEZ rather than being created in code (the `wifi_manager` screen creates its
Back button in code — we deliberately do **not** follow that here).

## Reference material

- EEZ JSON editing guide: [`_docs/eez-studio-json-reference.md`](../../../_docs/eez-studio-json-reference.md)
- Pattern to mirror: `wifi_manager` page + [`src/apps/wifi_manager/wifi_manager.cpp`](../../../src/apps/wifi_manager/wifi_manager.cpp)
  (named widgets → `objects.<name>`, design-time mockup replaced at runtime,
  a label made clickable via `LV_OBJ_FLAG_CLICKABLE` — the `scan_btn` pattern).
- Current behavior to preserve: [`src/apps/logs/logs.cpp`](../../../src/apps/logs/logs.cpp).

## Phase split

The user re-exports from EEZ Studio between the two phases.

- **Phase 1 — EEZ JSON edit (this change):** add the `logs` page to the
  `.eez-project`. → user re-exports → generated C appears in
  `src/ui/eez_export/screens.{h,c}` (`create_screen_logs()`, `objects.logs`,
  `objects.log_list`, `objects.log_back_btn`, `objects.log_clear_btn`,
  `SCREEN_ID_LOGS`).
- **Phase 2 — rewire `logs.cpp`:** refactor `logs_create()` to call
  `create_screen_logs()` and use the `objects.*` fields instead of building the
  tree by hand. Phase 2 is a **follow-up** after the export and is not part of
  this change.

## Phase 1 — EEZ page structure

New entry in `userPages`, `name: "logs"`, 320×480, `createAtStart: true`,
`deleteOnScreenUnload: false`. `components[0]` is the `LVGLScreenWidget`; its
`children[]`:

```
LVGLScreenWidget  (logs, 320×480)
├─ LVGLLabelWidget   name=log_back_btn   text="<LEFT> Back"   @(8,8)    text #60A5FA (accent_blue)
├─ LVGLLabelWidget   (title, unnamed)    text="LOGS"          top-mid   text #999999 (text_secondary)
├─ LVGLLabelWidget   name=log_clear_btn  text="<TRASH> Clear" top-right text #F59E0B (accent_amber)
└─ LVGLContainerWidget name=log_list     @(0,32) 320×448
     bg #000000, border_width 0, radius 0, pad_all 6, flex COLUMN, pad_row 2
     ├─ LVGLLabelWidget  "[INFO]  system boot ok"      text #F0F0F0   ┐
     ├─ LVGLLabelWidget  "[WARN]  battery low"         text #F59E0B   ├ design-time mockup only
     └─ LVGLLabelWidget  "[ERROR] i2c read timeout"    text #F87171   ┘
```

### Named widgets (the contract with Phase 2)

| EEZ `name`      | Type      | Wrapper uses it to…                                   |
|-----------------|-----------|------------------------------------------------------|
| `log_list`      | Container | `lv_obj_clean()` + append one label per log line     |
| `log_back_btn`  | Label     | make clickable → `screen_manager::pop()`             |
| `log_clear_btn` | Label     | make clickable → `logging::clear()`                  |

The title label is static ("LOGS") and stays unnamed. Auto `objN` names must
never be relied on (they shift on edits) — anything Phase 2 references is named.

### Widget details

- **Back / Clear are labels, not buttons.** Mirrors `scan_btn`: the wrapper
  adds `LV_OBJ_FLAG_CLICKABLE` and an event handler. Avoids assuming EEZ
  exposes a button widget type.
- **Symbols** stored as raw glyphs in JSON: `LV_SYMBOL_LEFT` (U+F053) for
  Back, `LV_SYMBOL_TRASH` (U+F2ED) for Clear. These are built-in LVGL symbols
  present in the exported font subset (same mechanism as `wifi_manager`'s
  `LV_SYMBOL_REFRESH`, which renders correctly — no "tofu rectangle").
- **Local styles** use `localStyles.definition.MAIN.DEFAULT`; colors as
  `#RRGGBB`; opacities are 0–255. Palette from
  [`src/ui/styles.h`](../../../src/ui/styles.h): bg_primary `#000000`,
  text_primary `#F0F0F0`, text_secondary `#999999`, accent_blue `#60A5FA`,
  accent_amber `#F59E0B`, accent_red `#F87171`.
- **Mockup lines** are design-time only; Phase 2 replaces them at runtime.

### Edit method

Programmatic Python edit of the `.eez-project` JSON (per the reference recipe):
generate `objID`s with `uuid.uuid4()`, copy the widget skeleton, append the new
page to `userPages`, then re-parse the file to confirm valid JSON. Do **not**
hand-edit `src/ui/eez_export/` (regenerated on export).

## Phase 2 (follow-up, for context — not implemented in this change)

Refactor `logs_create()`:
- call `create_screen_logs()`; apply `styles::style_screen` to `objects.logs`.
- configure `objects.log_list` as a vertical flex column; `lv_obj_clean()` the
  design-time mockup.
- keep the existing logic: `MAX_VISIBLE` cap, per-level color (`level_color`),
  auto-scroll to bottom, 400 ms `lv_timer` refresh keyed on
  `logging::revision()`, and `LV_EVENT_DELETE` cleanup of the timer.
- make `objects.log_back_btn` clickable → `screen_manager::pop()`.
- make `objects.log_clear_btn` clickable → `logging::clear()` + force rebuild.

`app_registry.cpp` already lists `{"Logs", "", app_category::debug,
logs_create}`; no registry change needed.

## Out of scope / non-goals

- No change to `app_registry.cpp` (entry already exists).
- No editing of generated `eez_export/` sources.
- No new logging backend behavior — UI/wiring parity only.

## Verification

- **Phase 1:** `python3 -c "import json; json.load(open(...))"` re-parses the
  edited `.eez-project` without error; the `logs` page and three named widgets
  are present in the JSON.
- **Post-export (user):** `objects.logs`, `objects.log_list`,
  `objects.log_back_btn`, `objects.log_clear_btn`, and `SCREEN_ID_LOGS` exist
  in `src/ui/eez_export/screens.h`.
- **Phase 2:** `~/.platformio/penv/bin/pio run` builds clean; on-device the
  Logs app opens, shows lines color-coded by level, auto-scrolls, Clear empties
  the list, Back returns to the launcher.

## Notes

This repo is not a git repository, so the design doc is written but not
committed.
