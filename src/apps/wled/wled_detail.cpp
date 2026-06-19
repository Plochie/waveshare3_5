#include "apps/wled/wled_detail.h"

#include "core/screen_manager.h"
#include "core/wled_client.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"

static String s_host;
static lv_obj_t *s_power_sw = nullptr;
static lv_obj_t *s_solid_sw = nullptr;
static lv_timer_t *s_flush_timer = nullptr;

// Current selection. Color is hue+saturation at full value; brightness is the
// separate WLED master brightness.
static uint16_t s_hue = 0;   // 0-359
static uint8_t s_sat = 0;    // 0-100 (LVGL HSV scale)
static uint8_t s_bri = 0;    // 0-255
static bool s_color_dirty = false;
static bool s_bri_dirty = false;

static void current_rgb(uint8_t &r, uint8_t &g, uint8_t &b)
{
  uint32_t c = lv_color_to_u32(lv_color_hsv_to_rgb(s_hue, s_sat, 100));
  r = (c >> 16) & 0xFF;
  g = (c >> 8) & 0xFF;
  b = c & 0xFF;
}

static void refresh_preview()
{
  uint8_t r, g, b;
  current_rgb(r, g, b);
  lv_color_t col = lv_color_make(r, g, b);
  lv_obj_set_style_bg_color(objects.live_preview, col, 0);
  // Knobs echo the chosen hue/color.
  lv_obj_set_style_bg_color(objects.hue_slider,
                            lv_color_hsv_to_rgb(s_hue, 100, 100), LV_PART_KNOB);
  lv_obj_set_style_bg_color(objects.sat_slider, col, LV_PART_KNOB);
}

static void style_slider_accent(lv_obj_t *slider, lv_color_t indicator)
{
  lv_obj_set_style_bg_color(slider, indicator, LV_PART_INDICATOR);
}

static void hue_cb(lv_event_t *e)
{
  s_hue = (uint16_t)lv_slider_get_value(objects.hue_slider);
  refresh_preview();
  s_color_dirty = true;
}

static void sat_cb(lv_event_t *e)
{
  // sat_slider is 0-255 in EEZ; convert to LVGL's 0-100 HSV scale.
  s_sat = (uint8_t)((int)lv_slider_get_value(objects.sat_slider) * 100 / 255);
  refresh_preview();
  s_color_dirty = true;
}

static void bri_cb(lv_event_t *e)
{
  s_bri = (uint8_t)lv_slider_get_value(objects.bri_slider);
  lv_label_set_text_fmt(objects.bri_label, "BRIGHTNESS - %u%%", (unsigned)((int)s_bri * 100 / 255));
  s_bri_dirty = true;
}

// Debounced flush: pushes at most one color + one brightness write per tick.
static void flush_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  if (s_color_dirty) {
    uint8_t r, g, b;
    current_rgb(r, g, b);
    wled_client::set_color(s_host.c_str(), r, g, b);
    s_color_dirty = false;
  }
  if (s_bri_dirty) {
    wled_client::set_brightness(s_host.c_str(), s_bri);
    s_bri_dirty = false;
  }
}

static void apply_rgb_selection(uint8_t r, uint8_t g, uint8_t b)
{
  lv_color_hsv_t hsv = lv_color_rgb_to_hsv(r, g, b);
  s_hue = hsv.h;
  s_sat = hsv.s;
  lv_slider_set_value(objects.hue_slider, s_hue, LV_ANIM_OFF);
  lv_slider_set_value(objects.sat_slider, (int)s_sat * 255 / 100, LV_ANIM_OFF);
  refresh_preview();
  s_color_dirty = true;
}

static void quick_color_cb(lv_event_t *e)
{
  uint32_t rgb = (uint32_t)(intptr_t)lv_event_get_user_data(e);
  apply_rgb_selection((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

static void power_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  wled_client::set_power(s_host.c_str(), lv_obj_has_state(s_power_sw, LV_STATE_CHECKED));
}

static void back_cb(lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); }

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_flush_timer) {
    lv_timer_delete(s_flush_timer);
    s_flush_timer = nullptr;
  }
  s_power_sw = nullptr;
  s_solid_sw = nullptr;
}

static void add_quick_color(lv_obj_t *parent, uint8_t r, uint8_t g, uint8_t b)
{
  lv_obj_t *sw = lv_obj_create(parent);
  lv_obj_set_size(sw, 28, 28);
  lv_obj_set_style_radius(sw, 14, 0);
  lv_obj_set_style_border_color(sw, styles::border(), 0);
  lv_obj_set_style_border_width(sw, 1, 0);
  lv_obj_set_style_bg_color(sw, lv_color_make(r, g, b), 0);
  lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(sw, 0, 0);
  uint32_t rgb = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  lv_obj_add_event_cb(sw, quick_color_cb, LV_EVENT_CLICKED, (void *)(intptr_t)rgb);
}

static lv_obj_t *make_header_switch(lv_obj_t *parent)
{
  lv_obj_t *sw = lv_switch_create(parent);
  lv_obj_set_size(sw, 44, 22);
  lv_obj_set_style_bg_color(sw, styles::accent_green(), LV_PART_INDICATOR | LV_STATE_CHECKED);
  return sw;
}

lv_obj_t *wled_detail_create(const char *host, const char *name, uint16_t leds)
{
  s_host = host ? host : "";
  s_color_dirty = false;
  s_bri_dirty = false;

  create_screen_wled_detail();
  lv_obj_add_style(objects.wled_detail, &styles::style_screen, 0);
  lv_obj_add_event_cb(objects.wled_detail, screen_delete_cb, LV_EVENT_DELETE, NULL);

  lv_obj_add_flag(objects.detail_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.detail_back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  lv_label_set_text(objects.detail_title, (name && name[0]) ? name : "WLED");

  // Header power switch (top-right).
  s_power_sw = make_header_switch(objects.wled_detail);
  lv_obj_align(s_power_sw, LV_ALIGN_TOP_RIGHT, -8, 6);
  lv_obj_add_event_cb(s_power_sw, power_cb, LV_EVENT_VALUE_CHANGED, NULL);

  style_slider_accent(objects.sat_slider, styles::accent_purple());
  style_slider_accent(objects.bri_slider, styles::accent_blue());
  lv_obj_add_event_cb(objects.hue_slider, hue_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(objects.sat_slider, sat_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(objects.bri_slider, bri_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Quick-color palette.
  lv_obj_set_flex_flow(objects.quick_colors, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(objects.quick_colors, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(objects.quick_colors, 0, 0);
  add_quick_color(objects.quick_colors, 255, 180, 107); // warm white
  add_quick_color(objects.quick_colors, 255, 255, 255); // white
  add_quick_color(objects.quick_colors, 255, 60, 60);   // red
  add_quick_color(objects.quick_colors, 80, 220, 120);  // green
  add_quick_color(objects.quick_colors, 80, 140, 255);  // blue
  add_quick_color(objects.quick_colors, 167, 139, 250); // purple
  add_quick_color(objects.quick_colors, 244, 114, 182); // pink

  // "Solid color mode" placeholder switch (effects deferred).
  s_solid_sw = make_header_switch(objects.solid_card);
  lv_obj_align(s_solid_sw, LV_ALIGN_RIGHT_MID, -12, 0);
  lv_obj_add_state(s_solid_sw, LV_STATE_CHECKED);

  // Pull initial state from the device.
  wled_client::status st;
  bool online = wled_client::get_state(host, st);
  if (online) {
    s_bri = st.bri;
    if (st.on) lv_obj_add_state(s_power_sw, LV_STATE_CHECKED);
    lv_slider_set_value(objects.bri_slider, s_bri, LV_ANIM_OFF);
    lv_label_set_text_fmt(objects.bri_label, "BRIGHTNESS - %u%%",
                          (unsigned)((int)s_bri * 100 / 255));
    apply_rgb_selection(st.r, st.g, st.b);
    s_color_dirty = false; // don't echo the freshly-read color back
  }
  lv_label_set_text_fmt(objects.detail_subtitle, "%s - %u LEDs - %s",
                        host, (unsigned)leds, online ? "online" : "offline");

  if (!s_flush_timer) {
    s_flush_timer = lv_timer_create(flush_cb, 150, NULL);
  }

  return objects.wled_detail;
}
