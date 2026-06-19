import json, uuid

P = "eez-studio/eez-studio3-5.eez-project"
data = json.load(open(P))


def oid():
    return str(uuid.uuid4())


def label(identifier, left, top, width, height_unit, text, styles_def,
          height=16, width_unit="px", left_unit="px", top_unit="px"):
    w = {
        "objID": oid(),
        "type": "LVGLLabelWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": left_unit, "topUnit": top_unit, "widthUnit": width_unit, "heightUnit": height_unit,
        "children": [],
        "widgetFlags": "CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal",
        "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": {"objID": oid(), "definition": {"MAIN": {"DEFAULT": styles_def}}},
        "group": "", "groupIndex": 0,
        "text": text, "textType": "literal", "longMode": "WRAP", "recolor": False,
    }
    if identifier:
        w["identifier"] = identifier
    return w


def container(identifier, left, top, width, height, styles_def, children=None,
               width_unit="px", height_unit="px", scroll_dir=""):
    w = {
        "objID": oid(),
        "type": "LVGLContainerWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": width_unit, "heightUnit": height_unit,
        "widgetFlags": "CLICKABLE|PRESS_LOCK|CLICK_FOCUSABLE|GESTURE_BUBBLE|SNAPPABLE|SCROLLABLE|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER",
        "hiddenFlagType": "literal",
        "clickableFlag": True,
        "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": scroll_dir, "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": {"objID": oid(), "definition": {"MAIN": {"DEFAULT": styles_def}}},
        "group": "", "groupIndex": 0,
        "children": children or [],
    }
    if identifier:
        w["identifier"] = identifier
    return w


# --- Header ---
back_btn = label("scanner_back_btn", 8, 8, 70, "content", " Back", {"text_color": "#60A5FA"})
title = label("scanner_title", 0, 8, 320, "content", "I2C / SPI SCANNER",
               {"text_color": "#999999", "text_align": "CENTER"})

# --- Mode toggle + scan ---
mode_i2c_btn = label("mode_i2c_btn", 8, 32, 50, "content", "I2C",
                      {"text_color": "#000000", "text_align": "CENTER", "bg_color": "#3ECF8E",
                       "bg_opa": 255, "radius": 6, "pad_top": 4, "pad_bottom": 4})
mode_spi_btn = label("mode_spi_btn", 62, 32, 50, "content", "SPI",
                      {"text_color": "#999999", "text_align": "CENTER", "bg_color": "#111115",
                       "bg_opa": 255, "radius": 6, "pad_top": 4, "pad_bottom": 4,
                       "border_color": "#2A2A2E", "border_width": 1})
scan_btn = label("scan_btn", 250, 36, 60, "content", " Scan",
                  {"text_color": "#3ECF8E", "text_align": "RIGHT"})

# --- Found-devices card ---
found_count_label = label("found_count_label", 8, 68, 304, "content", "FOUND — 3 DEVICES",
                           {"text_color": "#999999"})

device_rows_data = [
    ("0x3C", "SSD1306", "OLED"),
    ("0x68", "MPU6050", "IMU"),
    ("0x76", "BME280", "Env"),
]
device_rows = []
for i, (addr, name, kind) in enumerate(device_rows_data):
    row_top = 8 + i * 38
    addr_lbl = label(None, 12, 11, 60, "content", addr, {"text_color": "#3ECF8E"})
    name_lbl = label(None, 80, 11, 160, "content", name, {"text_color": "#F0F0F0"})
    kind_lbl = label(None, 232, 11, 64, "content", kind,
                      {"text_color": "#555555", "text_align": "RIGHT"})
    row = container(None, 8, row_top, 304, 34,
                     {"bg_color": "#111115", "bg_opa": 255, "radius": 8,
                      "border_color": "#2A2A2E", "border_width": 1,
                      "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0},
                     children=[addr_lbl, name_lbl, kind_lbl])
    device_rows.append(row)

device_list = container("device_list", 0, 90, 100, 130,
                         {"bg_opa": 0, "border_width": 0,
                          "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0},
                         children=device_rows, width_unit="%", scroll_dir="VER")

# --- Address map ---
addr_map_title = label("addr_map_title", 8, 228, 304, "content", "ADDRESS MAP — 0X00 → 0X7F",
                        {"text_color": "#999999"})

addr_grid = container("addr_grid", 8, 248, 304, 120,
                       {"bg_color": "#111115", "bg_opa": 255, "radius": 8,
                        "border_color": "#2A2A2E", "border_width": 1,
                        "pad_left": 4, "pad_top": 4, "pad_right": 4, "pad_bottom": 4})

addr_hint = label("addr_hint", 8, 376, 304, "content",
                   "tap address for datasheet · encoder scrolls list",
                   {"text_color": "#555555", "text_align": "CENTER"})

screen_widget = {
    "objID": oid(),
    "type": "LVGLScreenWidget",
    "left": 0, "top": 0, "width": 320, "height": 480,
    "customInputs": [], "customOutputs": [],
    "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
    "timeline": [], "eventHandlers": [],
    "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
    "children": [
        back_btn, title,
        mode_i2c_btn, mode_spi_btn, scan_btn,
        found_count_label, device_list,
        addr_map_title, addr_grid, addr_hint,
    ],
}

page = {
    "objID": oid(),
    "connectionLines": [],
    "localVariables": [],
    "componentGroups": [],
    "userProperties": [],
    "name": "i2c_spi_scanner",
    "left": 0, "top": 0, "width": 320, "height": 480,
    "isUsedAsUserWidget": False,
    "createAtStart": True,
    "deleteOnScreenUnload": False,
    "components": [screen_widget],
}

data["userPages"].append(page)

json.dump(data, open(P, "w"), indent=2)
open(P, "a").write("\n")

# Validate
json.load(open(P))
print("OK: added page", page["name"])
