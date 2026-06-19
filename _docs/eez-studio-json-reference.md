# EEZ Studio `.eez-project` JSON — editing reference

A working reference for **programmatically editing**
`eez-studio/eez-studio3-5.eez-project` (so Claude can make UI changes without
the EEZ GUI). Captures what we learned wiring the launcher / WiFi Manager
screens. LVGL **v9.2.2**, display **320×480**.

> Workflow: edit the `.eez-project` JSON → user re-exports from EEZ Studio →
> generated C lands in `src/ui/eez_export/`. **Never hand-edit `eez_export/`**
> (it's fully regenerated each export); put custom logic in the
> `src/apps/<name>/` wrapper.

---

## 1. Top-level shape

```jsonc
{
  "settings": { "general": {...}, "build": {...} },
  "variables": {...},
  "actions": [],
  "userPages":   [ ...screens... ],     // each entry = one screen
  "userWidgets": [ ...reusable widgets... ],  // e.g. status_bar
  "lvglStyles": {...}, "lvglGroups": {...},
  "fonts": [], "bitmaps": [], "colors": [], "themes": [...]
}
```

### settings.general (read-only facts that matter)
- `projectType: "lvgl"`, `lvglVersion: "9.2.2"`, `displayWidth/Height`.
- `darkTheme: true` is **editor-only** — it does NOT set any screen background.
  Exported screens have no bg of their own; the light LVGL default shows
  through. We apply `styles::style_screen` in the wrapper to get the dark bg.
- `colorFormat: "BGR"`.

### settings.build
- `destinationFolder: "../src/ui/eez_export"` — where export writes the C.
- `files[]` — the templates for each generated file (screens.h/.c, ui.h/.c, …).

---

## 2. Screens (`userPages`)

Each screen:
```jsonc
{
  "objID": "<uuid>",
  "name": "wifi_manager",          // -> create_screen_wifi_manager(), objects.wifi_manager
  "left":0,"top":0,"width":320,"height":480,
  "createAtStart": true, "deleteOnScreenUnload": false,
  "components": [ { "type":"LVGLScreenWidget", "children":[ ...widgets... ] } ]
}
```
- `components[0]` is always the `LVGLScreenWidget`; its `children[]` is the
  real widget tree.
- Screen `name` drives the generated `create_screen_<name>()`,
  `tick_screen_<name>()`, `ScreensEnum SCREEN_ID_<NAME>`, and `objects.<name>`.

---

## 3. The widget `identifier` property → `objects_t` fields  ⭐ most important

- A widget with a non-empty **`"identifier"`** is exported as an
  `objects.<identifier>` field (referenceable from the wrapper). Unnamed widgets
  get auto names `obj0, obj1, …` **in tree order**.
- ⚠️ **The key is `"identifier"`, NOT `"name"`.** `"name"` is used at the
  *page/userWidget* level (screens) — but individual **widgets** inside a screen
  name themselves with `"identifier"`. (Confirmed in the project file:
  `network_count`, `scan_btn`, `network_list`, `log_list`, `log_back_btn`,
  `log_clear_btn` all use `"identifier"`; no widget has a `"name"` key.) Setting
  `"name"` on a widget does nothing.
- ⚠️ **Auto `objN` numbers shift** whenever widgets are added/removed/named.
  Never reference `objN` long-term — **give an `identifier` to any widget the
  wrapper needs.** Must be a valid C identifier.
- Requires a **re-export** to appear in `screens.h`. The identifiers persist in
  the project file across exports.

---

## 4. Widget object schema

Common keys present on every widget (copy this skeleton, fill the rest):
```jsonc
{
  "objID": "<uuid4>",                 // unique; generate with uuid.uuid4()
  "type": "LVGLContainerWidget",      // see types below
  "left":0,"top":0,"width":100,"height":100,
  "leftUnit":"px","topUnit":"px","widthUnit":"px","heightUnit":"px", // or "%","content"
  "customInputs":[], "customOutputs":[],
  "style": { "objID":"<uuid>", "useStyle":"default",
             "conditionalStyles":[], "childStyles":[] },
  "timeline":[], "eventHandlers":[],
  "children":[],
  "widgetFlags":"CLICKABLE|PRESS_LOCK|...",
  "hiddenFlagType":"literal", "clickableFlagType":"literal",
  "checkedStateType":"literal", "disabledStateType":"literal",
  "states":"",
  "localStyles": { "objID":"<uuid>" },   // + "definition" for styling, see §6
  "group":"", "groupIndex":0
}
```
Type-specific extras:
- **LVGLContainerWidget**: also has `"clickableFlag": true`, `flagScrollbarMode`,
  `flagScrollDirection`, `scrollSnapX/Y`.
- **LVGLLabelWidget**: add `"text":"...", "textType":"literal",
  "longMode":"WRAP", "recolor":false`. `widthUnit:"content"` for auto-size.
  Also used for tappable "buttons" — a plain label with `LV_OBJ_FLAG_CLICKABLE`
  added in the wrapper (see §11).
- **LVGLSliderWidget**: also has `"clickableFlag": true`,
  `"widgetFlags":"CLICKABLE|CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_ON_FOCUS|SCROLL_WITH_ARROW|SNAPPABLE"`,
  and the range/value fields: `"min"/"max"` + `"minType"/"maxType":"literal"`,
  `"mode":"NORMAL"`, `"value"` + `"valueType":"literal"`, `"previewValue"`,
  `"valueLeft":0` + `"valueLeftType":"literal"`, `"previewValueLeft":0`,
  `"enableAnimation":false`. `localStyles` for a slider is just
  `{"objID":"<uuid>"}` (no `definition` needed — default LVGL slider styling
  is fine). Wire `LV_EVENT_VALUE_CHANGED` + `lv_slider_get_value()` in the
  wrapper to read drags.
- **LVGLUserWidgetWidget**: `"userWidgetPageName":"status_bar"`,
  `"userPropertyValues":{...}` — instantiates a reusable widget from
  `userWidgets`.
- Others seen: `LVGLScreenWidget`, `LVGLKeyboardWidget`.

---

## 5. Reusable widgets (`userWidgets`) + params

`status_bar` is a userWidget (a mini page, `isUsedAsUserWidget:true`). Screens
embed it via an `LVGLUserWidgetWidget` child. Generated as
`create_user_widget_status_bar(parent, startWidgetIndex)`. Template params use
`{{param}}` in label text (e.g. `"{{screen_title}}"`).

---

## 6. Styling: `localStyles.definition`

Per-widget local styles live under MAIN/DEFAULT (part/state):
```jsonc
"localStyles": {
  "objID":"<uuid>",
  "definition": { "MAIN": { "DEFAULT": {
    "bg_color":"#111115", "bg_opa":255,        // opa is 0–255 (or 0 = transparent)
    "radius":8,
    "border_color":"#2A2A2E", "border_width":1, "border_side":"BOTTOM", "border_opa":50,
    "pad_left":0,"pad_top":0,"pad_right":0,"pad_bottom":0,
    "text_color":"#F0F0F0", "text_align":"RIGHT", "text_font":"..."
  }}}
}
```
- Colors are `"#RRGGBB"` strings → exported as `lv_color_hex(0x......)`.
- Mirror the project's palette (see `src/ui/styles.h`): bg `#000000`, card
  `#111115`, border `#2A2A2E`, text `#F0F0F0/#999999/#555555`, accents
  green `#3ECF8E` / amber `#F59E0B` / red `#F87171` / blue `#60A5FA`.

---

## 7. Symbols & fonts (the "tofu rectangle" trap) ⭐

- The exported Montserrat font subset **does not include arbitrary Unicode**.
  Block chars like `▁▃▅▇` or `↻` render as **empty rectangles**.
- Use **built-in LVGL symbols** (present in the symbol font). In JSON, store the
  raw glyph codepoint; in C it maps to `LV_SYMBOL_*`:
  - `` = `LV_SYMBOL_REFRESH`  (we used for "Scan")
  - `` = `LV_SYMBOL_WIFI`
  - `` = `LV_SYMBOL_LEFT`, etc.
- For **custom graphics** (e.g. signal-strength bars) don't rely on glyphs —
  **compose from container rectangles** (4 thin `LVGLContainerWidget`s with
  increasing height + bg_color). This is exactly how the WiFi cards draw bars.

---

## 8. Static design content vs. dynamic runtime content ⭐

Demo content placed in EEZ (the 3–4 hard-coded network cards) is just a
**design-time mockup**. At runtime the wrapper does
`lv_obj_clean(objects.network_list)` and rebuilds real cards in code
(`make_network_card` in `src/apps/wifi_manager/wifi_manager.cpp`). So:
- Keep one or two representative cards in EEZ for visual layout reference.
- The real list is generated in C; the EEZ cards never actually show.
- When you change the card's *look* (separators, add a MAC line, etc.), update
  **both** the EEZ mockup (for the design) **and** the wrapper's builder (for
  what actually renders).

---

## 9. Safe programmatic-edit recipe (Python)

```python
import json, uuid
P = "eez-studio/eez-studio3-5.eez-project"
data = json.load(open(P))

def oid(): return str(uuid.uuid4())

# locate a screen
page = next(p for p in data["userPages"] if p["name"] == "wifi_manager")
screen = page["components"][0]            # LVGLScreenWidget
# walk screen["children"] ... find by name / text / top position, then mutate

json.dump(data, open(P,"w"), indent=2); open(P,"a").write("\n")
json.load(open(P))                        # re-parse to validate
```
Locate widgets by stable traits: widget **`identifier`** (or screen/userWidget
**`name`**), label **`text`**, or **position** (`top`/`left`). Always re-parse
to confirm valid JSON.
Container/label builder helpers we used live in this project's git history
(the WiFi card-building scripts) — reuse those skeletons.

---

## 10. Page header convention

Screens with a back action (`logs`, `file_explorer`, `wifi_manager`,
`settings`, ...) share a header layout — match it when adding new screens:
- `<screen>_back_btn` — `LVGLLabelWidget`, text `" Back"` (the
  `LV_SYMBOL_LEFT` codepoint ``, bytes `ef 81 93`), at
  `(8, 8, 70x16, heightUnit:"content")`, `text_color "#60A5FA"`
  (`accent_blue`). In the wrapper:
  `lv_obj_add_flag(objects.<screen>_back_btn, LV_OBJ_FLAG_CLICKABLE)` +
  `lv_obj_add_event_cb(..., LV_EVENT_CLICKED, ...)` → `screen_manager::pop()`.
  EEZ labels/containers are **not** clickable by default — always add the
  flag in code for anything tappable.
- A centered title/path label at `(0, 8, 320x16, heightUnit:"content")`,
  `text_color "#999999"` (`text_secondary`), `text_align "CENTER"`.
- An optional right-side action label/button at `top: 8`.
- Main content starts at `top: 32`.

This was used for `wifi_manager` (`wifi_back_btn`/`network_count`/`scan_btn`)
and `settings` (`settings_back_btn`/`settings_title`).

---

## 11. Gotchas checklist
- [ ] `darkTheme:true` ≠ dark screen bg — apply a style in the wrapper.
- [ ] Need to reference a widget? Give it an **`identifier`** (not `name`), then
      re-export. Don't trust `objN`.
- [ ] Glyph shows as a rectangle? It's not in the font — use `LV_SYMBOL_*` or
      build it from shapes.
- [ ] Edited the JSON? Tell the user to **re-export**; then verify the expected
      `objects.*` fields exist in `src/ui/eez_export/screens.h`.
- [ ] Changing card visuals → update the EEZ mockup **and** the C builder.
- [ ] `bg_opa`/`border_opa` are 0–255, not 0–100.
```
