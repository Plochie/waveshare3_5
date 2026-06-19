#include "apps/wled/wled.h"

#include "apps/wled/wled_add.h"
#include "apps/wled/wled_detail.h"
#include "core/screen_manager.h"
#include "core/wled_client.h"
#include "core/wled_store.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"

// Per-card widget references + last-known state, indexed in parallel with the
// rebuilt device list.
struct card_ref {
  lv_obj_t *card;
  lv_obj_t *sw;      // power switch
  lv_obj_t *swatch;  // color dot
  lv_obj_t *fill;    // brightness bar fill
  lv_obj_t *pct;     // brightness percent label
  lv_obj_t *name;
  lv_obj_t *detail;
  String host;
  String dev_name;
  uint16_t leds;
  bool on;
  uint8_t bri;
  bool known; // a get_state has succeeded at least once
};

static card_ref s_cards[wled_store::MAX_DEVICES];
static size_t s_count = 0;
static lv_obj_t *s_master_sw = nullptr;
static lv_timer_t *s_timer = nullptr;
static size_t s_refresh_idx = 0;

static const int BAR_W = 180;

static void style_switch(lv_obj_t *sw)
{
  lv_obj_set_size(sw, 44, 22);
  lv_obj_set_style_bg_color(sw, styles::accent_green(),
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
}

// Recomputes the "All Lights" summary + master switch from known card states.
static void update_summary()
{
  size_t on_count = 0;
  uint32_t bri_sum = 0;
  size_t known = 0;
  for (size_t i = 0; i < s_count; i++) {
    if (!s_cards[i].known) continue;
    known++;
    if (s_cards[i].on) {
      on_count++;
      bri_sum += (uint32_t)s_cards[i].bri * 100 / 255;
    }
  }
  if (known == 0) {
    lv_label_set_text_fmt(objects.all_lights_summary, "%u devices", (unsigned)s_count);
  } else {
    unsigned avg = on_count ? (unsigned)(bri_sum / on_count) : 0;
    lv_label_set_text_fmt(objects.all_lights_summary, "%u of %u on - avg %u%%",
                          (unsigned)on_count, (unsigned)s_count, avg);
  }
  if (s_master_sw) {
    if (on_count > 0) lv_obj_add_state(s_master_sw, LV_STATE_CHECKED);
    else lv_obj_clear_state(s_master_sw, LV_STATE_CHECKED);
  }
}

// Applies on/bri/color to a card's visuals (no events fired).
static void apply_card_visual(size_t idx)
{
  card_ref &c = s_cards[idx];
  bool on = c.on;

  if (on) lv_obj_add_state(c.sw, LV_STATE_CHECKED);
  else lv_obj_clear_state(c.sw, LV_STATE_CHECKED);

  lv_obj_set_style_text_color(c.name, on ? styles::text_primary() : styles::text_secondary(), 0);

  int w = (int)c.bri * BAR_W / 255;
  lv_obj_set_width(c.fill, on ? w : 0);
  lv_label_set_text_fmt(c.pct, "%u%%", on ? (unsigned)((int)c.bri * 100 / 255) : 0);
}

static void update_card_color(size_t idx, uint8_t r, uint8_t g, uint8_t b, bool on)
{
  lv_color_t col = on ? lv_color_make(r, g, b) : styles::border();
  lv_obj_set_style_bg_color(s_cards[idx].swatch, col, 0);
}

static void switch_cb(lv_event_t *e)
{
  size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
  if (idx >= s_count) return;
  bool on = lv_obj_has_state(s_cards[idx].sw, LV_STATE_CHECKED);
  wled_client::set_power(s_cards[idx].host.c_str(), on);
  s_cards[idx].on = on;
  s_cards[idx].known = true;
  apply_card_visual(idx);
  update_summary();
}

static void master_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  bool on = lv_obj_has_state(s_master_sw, LV_STATE_CHECKED);
  for (size_t i = 0; i < s_count; i++) {
    wled_client::set_power(s_cards[i].host.c_str(), on);
    s_cards[i].on = on;
    s_cards[i].known = true;
    apply_card_visual(i);
  }
  update_summary();
}

static void card_click_cb(lv_event_t *e)
{
  size_t idx = (size_t)(intptr_t)lv_event_get_user_data(e);
  if (idx >= s_count) return;
  screen_manager::push(wled_detail_create(s_cards[idx].host.c_str(),
                                          s_cards[idx].dev_name.c_str(),
                                          s_cards[idx].leds));
}

static void make_card(lv_obj_t *parent, size_t idx, const wled_store::device &dev)
{
  card_ref &c = s_cards[idx];
  c.host = dev.host;
  c.dev_name = dev.name;
  c.leds = dev.leds;
  c.on = false;
  c.bri = 0;
  c.known = false;

  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, 304, 64);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_style_bg_color(card, styles::bg_card(), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(card, 8, 0);
  lv_obj_set_style_border_color(card, styles::border(), 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_pad_all(card, 0, 0);
  lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(card, card_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);
  c.card = card;

  c.swatch = lv_obj_create(card);
  lv_obj_set_size(c.swatch, 16, 16);
  lv_obj_set_pos(c.swatch, 12, 12);
  lv_obj_remove_flag(c.swatch, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_radius(c.swatch, 8, 0);
  lv_obj_set_style_border_width(c.swatch, 0, 0);
  lv_obj_set_style_bg_color(c.swatch, styles::border(), 0);
  lv_obj_set_style_bg_opa(c.swatch, LV_OPA_COVER, 0);

  c.name = lv_label_create(card);
  lv_obj_set_pos(c.name, 40, 9);
  lv_obj_set_width(c.name, 180);
  lv_label_set_long_mode(c.name, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(c.name, styles::text_secondary(), 0);
  lv_label_set_text(c.name, dev.name.c_str());

  c.detail = lv_label_create(card);
  lv_obj_set_pos(c.detail, 40, 31);
  lv_obj_set_width(c.detail, 180);
  lv_label_set_long_mode(c.detail, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(c.detail, styles::text_muted(), 0);
  lv_label_set_text_fmt(c.detail, "%s - %u LEDs", dev.host.c_str(), (unsigned)dev.leds);

  // Brightness bar: a track with a colored fill.
  lv_obj_t *track = lv_obj_create(card);
  lv_obj_set_size(track, BAR_W, 4);
  lv_obj_set_pos(track, 40, 52);
  lv_obj_remove_flag(track, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_color(track, styles::border(), 0);
  lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(track, 2, 0);
  lv_obj_set_style_border_width(track, 0, 0);
  lv_obj_set_style_pad_all(track, 0, 0);

  c.fill = lv_obj_create(track);
  lv_obj_set_size(c.fill, 0, 4);
  lv_obj_set_pos(c.fill, 0, 0);
  lv_obj_remove_flag(c.fill, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_color(c.fill, styles::accent_blue(), 0);
  lv_obj_set_style_bg_opa(c.fill, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(c.fill, 2, 0);
  lv_obj_set_style_border_width(c.fill, 0, 0);
  lv_obj_set_style_pad_all(c.fill, 0, 0);

  c.pct = lv_label_create(card);
  lv_obj_set_pos(c.pct, 228, 46);
  lv_obj_set_width(c.pct, 32);
  lv_obj_set_style_text_align(c.pct, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_text_color(c.pct, styles::text_muted(), 0);
  lv_label_set_text(c.pct, "0%");

  c.sw = lv_switch_create(card);
  style_switch(c.sw);
  lv_obj_set_pos(c.sw, 250, 9);
  lv_obj_add_event_cb(c.sw, switch_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)idx);
}

static void rebuild_list()
{
  wled_store::device devs[wled_store::MAX_DEVICES];
  s_count = wled_store::load(devs, wled_store::MAX_DEVICES);

  lv_obj_clean(objects.wled_device_list);

  if (s_count == 0) {
    lv_obj_t *l = lv_label_create(objects.wled_device_list);
    lv_obj_set_style_text_color(l, styles::text_muted(), 0);
    lv_label_set_text(l, "No devices yet - tap + Add to discover one");
    lv_label_set_text(objects.devices_label, "DEVICES - 0");
    update_summary();
    return;
  }

  for (size_t i = 0; i < s_count; i++) {
    make_card(objects.wled_device_list, i, devs[i]);
  }
  lv_label_set_text_fmt(objects.devices_label, "DEVICES - %u", (unsigned)s_count);
  s_refresh_idx = 0;
  update_summary();
}

// Fetches one device's live state per tick (staggered to keep the UI smooth).
static void refresh_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  if (s_count == 0) return;
  size_t idx = s_refresh_idx % s_count;
  s_refresh_idx++;

  wled_client::status st;
  if (!wled_client::get_state(s_cards[idx].host.c_str(), st)) {
    return; // unreachable / offline: leave card as-is
  }
  s_cards[idx].on = st.on;
  s_cards[idx].bri = st.bri;
  s_cards[idx].known = true;
  apply_card_visual(idx);
  update_card_color(idx, st.r, st.g, st.b, st.on);
  update_summary();
}

static void screen_loaded_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  rebuild_list();
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_timer) {
    lv_timer_delete(s_timer);
    s_timer = nullptr;
  }
  s_count = 0;
  s_master_sw = nullptr;
}

static void back_cb(lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); }

static void add_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  // Opens the mDNS auto-discovery flow (which offers manual entry as a
  // fallback).
  screen_manager::push(wled_add_create());
}

lv_obj_t *wled_create()
{
  create_screen_wled_lights();

  lv_obj_add_style(objects.wled_lights, &styles::style_screen, 0);
  lv_obj_add_event_cb(objects.wled_lights, screen_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
  lv_obj_add_event_cb(objects.wled_lights, screen_delete_cb, LV_EVENT_DELETE, NULL);

  lv_obj_add_flag(objects.wled_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.wled_back_btn, back_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(objects.wled_add_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.wled_add_btn, add_cb, LV_EVENT_CLICKED, NULL);

  // "All Lights" master switch, parented into the EEZ card.
  s_master_sw = lv_switch_create(objects.all_lights_card);
  style_switch(s_master_sw);
  lv_obj_align(s_master_sw, LV_ALIGN_RIGHT_MID, -12, 0);
  lv_obj_add_event_cb(s_master_sw, master_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Device list scrolls as a flex column.
  lv_obj_set_flex_flow(objects.wled_device_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(objects.wled_device_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(objects.wled_device_list, 8, 0);

  if (!s_timer) {
    s_timer = lv_timer_create(refresh_cb, 600, NULL);
  }

  return objects.wled_lights;
}
