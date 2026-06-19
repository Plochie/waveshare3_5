import json, uuid

P = "eez-studio/eez-studio3-5.eez-project"
data = json.load(open(P))


def oid():
    return str(uuid.uuid4())


def style(defn):
    return {"objID": oid(), "definition": {"MAIN": {"DEFAULT": defn}}}


# Palette
TXT = "#F0F0F0"; SEC = "#999999"; MUT = "#555555"
GREEN = "#3ECF8E"; AMBER = "#F59E0B"; RED = "#F87171"; BLUE = "#60A5FA"; PURPLE = "#A78BFA"
CARD = "#111115"; BORDER = "#2A2A2E"; SURFACE = "#1A1A1F"


def label(identifier, left, top, width, text, styles_def,
          height=16, height_unit="content", width_unit="px", children=None):
    w = {
        "objID": oid(), "type": "LVGLLabelWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": width_unit, "heightUnit": height_unit,
        "children": children or [],
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
               width_unit="px", height_unit="px", hidden=False):
    flags = "CLICKABLE|PRESS_LOCK|CLICK_FOCUSABLE|GESTURE_BUBBLE|SNAPPABLE|SCROLLABLE|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER"
    if hidden:
        flags = "HIDDEN|" + flags
    w = {
        "objID": oid(), "type": "LVGLContainerWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": width_unit, "heightUnit": height_unit,
        "widgetFlags": flags,
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


def keyboard(identifier, left, top, width, height, mode="TEXT_LOWER"):
    return {
        "objID": oid(), "type": "LVGLKeyboardWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
        "children": [],
        "widgetFlags": "GESTURE_BUBBLE|PRESS_LOCK|SCROLLABLE|SCROLL_CHAIN_HOR|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal", "clickableFlag": True, "clickableFlagType": "literal",
        "flagScrollbarMode": "", "flagScrollDirection": "", "scrollSnapX": "", "scrollSnapY": "",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": {"objID": oid(), "definition": {"MAIN": {"DEFAULT": {"align": "DEFAULT"}}}},
        "group": "", "groupIndex": 0,
        "mode": mode,
    }


def slider(identifier, left, top, width, vmin, vmax, value, height=12):
    return {
        "objID": oid(), "type": "LVGLSliderWidget",
        "left": left, "top": top, "width": width, "height": height,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "identifier": identifier,
        "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
        "children": [],
        "widgetFlags": "CLICKABLE|CLICK_FOCUSABLE|GESTURE_BUBBLE|PRESS_LOCK|SCROLL_CHAIN_VER|SCROLL_ELASTIC|SCROLL_MOMENTUM|SCROLL_ON_FOCUS|SCROLL_WITH_ARROW|SNAPPABLE",
        "hiddenFlagType": "literal", "clickableFlag": True, "clickableFlagType": "literal",
        "checkedStateType": "literal", "disabledStateType": "literal", "states": "",
        "localStyles": {"objID": oid()},
        "group": "", "groupIndex": 0,
        "min": vmin, "minType": "literal", "max": vmax, "maxType": "literal",
        "mode": "NORMAL", "value": value, "valueType": "literal", "previewValue": value,
        "valueLeft": 0, "valueLeftType": "literal", "previewValueLeft": 0,
        "enableAnimation": False,
    }


# Style presets
def card_style(extra=None):
    s = {"bg_color": CARD, "bg_opa": 255, "radius": 8, "border_color": BORDER,
         "border_width": 1, "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0}
    if extra:
        s.update(extra)
    return s


def transparent_style():
    return {"bg_opa": 0, "border_width": 0,
            "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0}


def button_label(identifier, left, top, width, text, txt_color, height=30):
    return label(identifier, left, top, width, text,
                 {"text_color": txt_color, "text_align": "CENTER", "bg_color": CARD,
                  "bg_opa": 255, "radius": 8, "border_color": BORDER, "border_width": 1,
                  "pad_top": 7, "pad_bottom": 7}, height=height, height_unit="px")


def back_btn(identifier):
    return label(identifier, 8, 8, 70, " Back", {"text_color": BLUE})


def title(identifier, text):
    return label(identifier, 0, 8, 320, text, {"text_color": SEC, "text_align": "CENTER"})


def make_page(name, children):
    screen = {
        "objID": oid(), "type": "LVGLScreenWidget",
        "left": 0, "top": 0, "width": 320, "height": 480,
        "customInputs": [], "customOutputs": [],
        "style": {"objID": oid(), "useStyle": "default", "conditionalStyles": [], "childStyles": []},
        "timeline": [], "eventHandlers": [],
        "leftUnit": "px", "topUnit": "px", "widthUnit": "px", "heightUnit": "px",
        "children": children,
    }
    return {
        "objID": oid(), "connectionLines": [], "localVariables": [],
        "componentGroups": [], "userProperties": [],
        "name": name, "left": 0, "top": 0, "width": 320, "height": 480,
        "isUsedAsUserWidget": False, "createAtStart": True, "deleteOnScreenUnload": False,
        "components": [screen],
    }


# ===================== Screen 1: wled_lights =====================
all_lights_card = container("all_lights_card", 8, 34, 304, 56, card_style(), children=[
    label("all_lights_name", 12, 9, 160, "All Lights", {"text_color": TXT}),
    label("all_lights_summary", 12, 31, 220, "3 of 4 on - avg 62%", {"text_color": SEC}),
])
# one design-time mockup device card (cleaned + rebuilt in C at runtime)
mock_card = container(None, 0, 0, 304, 64, card_style(), children=[
    container(None, 12, 12, 16, 16, {"bg_color": PURPLE, "bg_opa": 255, "radius": 8,
                                      "border_width": 0, "pad_left": 0, "pad_top": 0,
                                      "pad_right": 0, "pad_bottom": 0}),
    label(None, 40, 10, 180, "Living Room Strip", {"text_color": TXT}),
    label(None, 40, 32, 200, "192.168.1.42 - 144 LEDs", {"text_color": MUT}),
])
device_list = container("device_list", 8, 122, 304, 300, transparent_style(), children=[mock_card])

wled_lights = make_page("wled_lights", [
    back_btn("wled_back_btn"), title("wled_title", "WLED LIGHTS"),
    label("wled_add_btn", 250, 8, 60, "+ Add", {"text_color": GREEN, "text_align": "RIGHT"}),
    all_lights_card,
    label("devices_label", 8, 100, 200, "DEVICES - 4", {"text_color": SEC}),
    device_list,
    label("wled_hint", 8, 450, 304, "tap device for controls - + to discover",
          {"text_color": MUT, "text_align": "CENTER"}),
])

# ===================== Screen 2: wled_detail =====================
live_preview = container("live_preview", 8, 52, 304, 22,
                          {"bg_color": PURPLE, "bg_opa": 255, "radius": 6, "border_width": 0,
                           "pad_left": 0, "pad_top": 0, "pad_right": 0, "pad_bottom": 0},
                          children=[
    label("live_preview_label", 0, 3, 304, "LIVE PREVIEW",
          {"text_color": "#000000", "text_align": "CENTER"}),
])
solid_card = container("solid_card", 8, 276, 304, 52, card_style(), children=[
    label("solid_label", 12, 8, 200, "Solid color mode", {"text_color": TXT}),
    label("solid_sub", 12, 30, 240, "effects/presets - coming in a future update",
          {"text_color": MUT}),
])
wled_detail = make_page("wled_detail", [
    back_btn("detail_back_btn"), title("detail_title", "DEVICE"),
    label("detail_subtitle", 0, 32, 320, "host - 0 LEDs - online",
          {"text_color": SEC, "text_align": "CENTER"}),
    live_preview,
    label("hue_label", 8, 84, 200, "HUE", {"text_color": SEC}),
    slider("hue_slider", 8, 102, 304, 0, 359, 180),
    label("sat_label", 8, 124, 200, "SATURATION", {"text_color": SEC}),
    slider("sat_slider", 8, 142, 304, 0, 255, 255),
    label("bri_label", 8, 164, 200, "BRIGHTNESS - 75%", {"text_color": SEC}),
    slider("bri_slider", 8, 182, 304, 0, 255, 191),
    label("quick_label", 8, 206, 200, "QUICK COLORS", {"text_color": SEC}),
    container("quick_colors", 8, 226, 304, 40, transparent_style()),
    solid_card,
    label("detail_hint", 8, 440, 304, "drag sliders to adjust - encoder -> brightness",
          {"text_color": MUT, "text_align": "CENTER"}),
])

# ===================== Screen 3: wled_add =====================
scan_section = container("scan_section", 0, 32, 320, 400, transparent_style(), children=[
    label("scan_status", 0, 210, 320, "Scanning network...",
          {"text_color": TXT, "text_align": "CENTER"}),
    label("scan_sub", 0, 234, 320, "Looking for WLED devices via mDNS",
          {"text_color": MUT, "text_align": "CENTER"}),
    label("scan_timer", 0, 258, 320, "0.0s / 5s", {"text_color": MUT, "text_align": "CENTER"}),
    label("scan_skip_btn", 60, 300, 200, "Skip - add manually instead",
          {"text_color": BLUE, "text_align": "CENTER"}),
])
results_section = container("results_section", 0, 32, 320, 400, transparent_style(),
                             hidden=True, children=[
    label("results_banner", 8, 8, 304, "Found 2 devices on network",
          {"text_color": GREEN, "text_align": "CENTER", "bg_color": "#10231A", "bg_opa": 255,
           "radius": 8, "pad_top": 6, "pad_bottom": 6}, height=28, height_unit="px"),
    container("results_list", 8, 44, 304, 340, transparent_style(), children=[
        container(None, 0, 0, 304, 56, card_style(), children=[
            container(None, 12, 16, 16, 16, {"bg_color": "#EAD9A0", "bg_opa": 255, "radius": 8,
                                             "border_width": 0, "pad_left": 0, "pad_top": 0,
                                             "pad_right": 0, "pad_bottom": 0}),
            label(None, 40, 9, 200, "wled-office", {"text_color": TXT}),
            label(None, 40, 31, 240, "192.168.1.74 - 60 LEDs - FW 0.14.4", {"text_color": MUT}),
        ]),
    ]),
])
empty_section = container("empty_section", 0, 32, 320, 400, transparent_style(),
                           hidden=True, children=[
    label("empty_title", 0, 120, 320, "No WLED devices found",
          {"text_color": TXT, "text_align": "CENTER"}),
    label("empty_sub", 0, 144, 320, "Scan completed - 0 devices responded",
          {"text_color": MUT, "text_align": "CENTER"}),
    label("empty_tips", 16, 184, 288,
          "TRY THIS:\n- Check the WLED device is powered on\n- Confirm it's on the same WiFi\n- Some routers block mDNS across bands",
          {"text_color": SEC}, height=80, height_unit="px"),
])
wled_add = make_page("wled_add", [
    back_btn("add_back_btn"), title("add_title", "ADD WLED DEVICE"),
    scan_section, results_section, empty_section,
    button_label("scan_again_btn", 8, 440, 150, "Scan again", SEC),
    button_label("add_manual_btn", 162, 440, 150, "+ Add manually", GREEN),
])

# ===================== Screen 4: wled_add_manual =====================
manual_result = container("manual_result", 8, 140, 304, 56, card_style(), hidden=True, children=[
    label("manual_result_name", 12, 9, 280, "wled-garage", {"text_color": GREEN}),
    label("manual_result_info", 12, 31, 280, "120 LEDs - FW 0.14.4 - 84ms", {"text_color": MUT}),
])
wled_add_manual = make_page("wled_add_manual", [
    back_btn("manual_back_btn"), title("manual_title", "ADD MANUALLY"),
    label("manual_input_label", 8, 32, 304, "IP ADDRESS OR HOSTNAME", {"text_color": SEC}),
    textarea("manual_input", 8, 52, 304, 40, "192.168.1.x",
             {"bg_color": CARD, "bg_opa": 255, "border_color": BORDER, "border_width": 1,
              "radius": 8, "text_color": TXT, "pad_left": 8, "pad_top": 8,
              "pad_right": 8, "pad_bottom": 8}),
    button_label("manual_test_btn", 8, 100, 304, "Test Connection", BLUE, height=32),
    manual_result,
    button_label("manual_add_btn", 8, 206, 304, "Add Device", GREEN, height=36),
    keyboard("manual_keyboard", 0, 353, 320, 127, mode="NUMBER"),
])

# Append all pages
for page in (wled_lights, wled_detail, wled_add, wled_add_manual):
    data["userPages"].append(page)

json.dump(data, open(P, "w"), indent=2)
open(P, "a").write("\n")
json.load(open(P))  # validate
print("OK: added wled_lights, wled_detail, wled_add, wled_add_manual")
