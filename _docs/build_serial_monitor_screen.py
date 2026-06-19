import json, uuid

P = "eez-studio/eez-studio3-5.eez-project"
data = json.load(open(P))


def oid():
    return str(uuid.uuid4())


def style(defn):
    return {"objID": oid(), "definition": {"MAIN": {"DEFAULT": defn}}}


def label(identifier, left, top, width, text, styles_def,
          height=16, height_unit="content", width_unit="px"):
    w = {
        "objID": oid(), "type": "LVGLLabelWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": width_unit, "heightUnit": height_unit,
        "children": [],
        "widgetFlags": "CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal", "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": style(styles_def),
        "group": "", "groupIndex": 0,
        "text": text, "textType": "literal", "longMode": "WRAP", "recolor": False,
    }
    if identifier:
        w["identifier"] = identifier
    return w


def container(identifier, left, top, width, height, styles_def, children=None,
               width_unit="px", height_unit="px"):
    w = {
        "objID": oid(), "type": "LVGLContainerWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": width_unit, "heightUnit": height_unit,
        "widgetFlags": "CLICKABLE|PRESS_LOCK|CLICK_FOCUSABLE|GESTURE_BUBBLE|SNAPPABLE|SCROLLABLE|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER",
        "hiddenFlagType": "literal", "clickableFlag": True, "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": style(styles_def),
        "group": "", "groupIndex": 0,
        "children": children or [],
    }
    if identifier:
        w["identifier"] = identifier
    return w


def textarea(identifier, left, top, width, height, placeholder, styles_def):
    return {
        "objID": oid(), "type": "LVGLTextareaWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
        "children": [],
        "widgetFlags": "CLICKABLE|PRESS_LOCK|CLICK_FOCUSABLE|GESTURE_BUBBLE|SNAPPABLE|SCROLLABLE|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER",
        "hiddenFlagType": "literal", "clickableFlag": True, "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": style(styles_def),
        "group": "", "groupIndex": 0,
        "text": "", "placeholder": placeholder, "oneLineMode": True,
        "passwordMode": False, "acceptedCharacters": "",
    }


def keyboard(identifier, left, top, width, height):
    return {
        "objID": oid(), "type": "LVGLKeyboardWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
        "children": [],
        # Start HIDDEN; the wrapper toggles it on textarea focus.
        "widgetFlags": "HIDDEN|GESTURE_BUBBLE|PRESS_LOCK|SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal", "clickableFlag": True, "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": {"objID": oid(), "definition": {"MAIN": {"DEFAULT": {"align": "DEFAULT"}}}},
        "group": "", "groupIndex": 0,
        "mode": "TEXT_LOWER",
    }


# Palette
TXT = "#F0F0F0"; SEC = "#999999"; MUT = "#555555"
GREEN = "#3ECF8E"; AMBER = "#F59E0B"; RED = "#F87171"; BLUE = "#60A5FA"
CARD = "#111115"; BORDER = "#2A2A2E"; SURFACE = "#1A1A1F"


def tag(identifier, left, top, width, text, txt_color):
    # A small pill-style tappable label (config cyclers / macro buttons).
    return label(identifier, left, top, width, text,
                 {"text_color": txt_color, "text_align": "CENTER", "bg_color": CARD,
                  "bg_opa": 255, "radius": 6, "border_color": BORDER, "border_width": 1,
                  "pad_top": 3, "pad_bottom": 3, "pad_left": 2, "pad_right": 2},
                 height=22, height_unit="px")


# --- Header ---
back_btn = label("serial_back_btn", 8, 8, 70, " Back", {"text_color": BLUE})
title = label("serial_title", 0, 8, 320, "SERIAL MONITOR",
               {"text_color": SEC, "text_align": "CENTER"})

# --- Config bar (top=32) ---
baud_btn = tag("baud_btn", 8, 30, 64, "115200", GREEN)
framing_btn = tag("framing_btn", 78, 30, 44, "8N1", SEC)
rx_dot = container("rx_dot", 134, 38, 8, 8,
                    {"bg_color": SURFACE, "bg_opa": 255, "radius": 4, "border_width": 0,
                     "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0})
rx_label = label("rx_label", 146, 34, 24, "RX", {"text_color": SEC})
tx_dot = container("tx_dot", 178, 38, 8, 8,
                    {"bg_color": SURFACE, "bg_opa": 255, "radius": 4, "border_width": 0,
                     "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0})
tx_label = label("tx_label", 190, 34, 24, "TX", {"text_color": SEC})
lineend_btn = tag("lineend_btn", 248, 30, 64, "CR+LF", BLUE)

# --- Terminal output (top=56, ~300px) ---
mock_lines = [
    ("[00:00:01] boot", MUT),
    ("[00:00:01] WiFi OK", GREEN),
    ("[00:00:02] T=28.4 H=67", TXT),
    ("[00:00:05] WARN: Vbat low", AMBER),
    ("[00:00:09] ERR: timeout", RED),
    ("> _", BLUE),
]
term_children = []
for i, (txt, col) in enumerate(mock_lines):
    term_children.append(label(None, 8, 8 + i * 20, 288, txt, {"text_color": col}))
term_output = container("term_output", 8, 56, 304, 300,
                         {"bg_color": CARD, "bg_opa": 255, "radius": 8,
                          "border_color": BORDER, "border_width": 1,
                          "pad_left": 4, "pad_top": 4, "pad_right": 4, "pad_bottom": 4},
                         children=term_children)

# --- Input row (top=360) ---
term_input = textarea("term_input", 8, 360, 228, 36, "type a command…",
                       {"bg_color": CARD, "bg_opa": 255, "border_color": BORDER,
                        "border_width": 1, "radius": 8, "text_color": TXT,
                        "pad_left": 8, "pad_top": 8, "pad_right": 8, "pad_bottom": 8})
send_btn = label("send_btn", 244, 366, 68, "Send",
                  {"text_color": GREEN, "text_align": "CENTER", "bg_color": CARD,
                   "bg_opa": 255, "radius": 8, "border_color": BORDER, "border_width": 1,
                   "pad_top": 5, "pad_bottom": 5}, height=24, height_unit="px")

# --- Macro row (top=404) ---
macro_at_btn = tag("macro_at_btn", 8, 404, 40, "AT", SEC)
macro_rst_btn = tag("macro_rst_btn", 54, 404, 46, "RST", SEC)
macro_help_btn = tag("macro_help_btn", 106, 404, 48, "help", SEC)
macro_clr_btn = tag("macro_clr_btn", 160, 404, 40, "clr", SEC)
macro_add_btn = tag("macro_add_btn", 206, 404, 76, "+ macro", MUT)

# --- On-screen keyboard (hidden) ---
serial_keyboard = keyboard("serial_keyboard", 0, 353, 320, 127)

screen_widget = {
    "objID": oid(), "type": "LVGLScreenWidget",
    "left": 0, "top": 0, "width": 320, "height": 480,
    "customInputs": [], "customOutputs": [],
    "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
    "timeline": [], "eventHandlers": [],
    "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
    "children": [
        back_btn, title,
        baud_btn, framing_btn, rx_dot, rx_label, tx_dot, tx_label, lineend_btn,
        term_output,
        term_input, send_btn,
        macro_at_btn, macro_rst_btn, macro_help_btn, macro_clr_btn, macro_add_btn,
        serial_keyboard,
    ],
}

page = {
    "objID": oid(), "connectionLines": [], "localVariables": [],
    "componentGroups": [], "userProperties": [],
    "name": "serial_monitor",
    "left": 0, "top": 0, "width": 320, "height": 480,
    "isUsedAsUserWidget": False, "createAtStart": True, "deleteOnScreenUnload": False,
    "components": [screen_widget],
}

data["userPages"].append(page)

json.dump(data, open(P, "w"), indent=2)
open(P, "a").write("\n")
json.load(open(P))  # validate
print("OK: added page serial_monitor with",
      len(screen_widget["children"]), "top-level widgets")
