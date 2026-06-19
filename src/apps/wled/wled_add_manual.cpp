#include "apps/wled/wled_add_manual.h"

#include "core/screen_manager.h"
#include "core/wled_client.h"
#include "core/wled_store.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"

// Result of the most recent successful Test Connection (gates Add Device).
static bool s_valid = false;
static String s_host;
static String s_name;
static uint16_t s_leds = 0;

static void set_add_enabled(bool enabled)
{
  lv_obj_set_style_text_color(objects.manual_add_btn,
                              enabled ? styles::accent_green() : styles::text_muted(), 0);
}

static void show_result(bool ok, const char *name, const char *info)
{
  lv_obj_clear_flag(objects.manual_result, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_border_color(objects.manual_result,
                                ok ? styles::accent_green() : styles::accent_red(), 0);
  lv_obj_set_style_text_color(objects.manual_result_name,
                              ok ? styles::accent_green() : styles::accent_red(), 0);
  lv_label_set_text(objects.manual_result_name, name);
  lv_label_set_text(objects.manual_result_info, info);
}

static void test_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  String host = lv_textarea_get_text(objects.manual_input);
  host.trim();
  if (host.length() == 0) {
    s_valid = false;
    set_add_enabled(false);
    show_result(false, "Enter an IP or hostname", "e.g. 192.168.1.74");
    return;
  }

  wled_client::info inf;
  if (wled_client::get_info(host.c_str(), inf)) {
    s_valid = true;
    s_host = host;
    s_name = inf.name;
    s_leds = inf.leds;
    set_add_enabled(true);
    String detail = String(inf.leds) + " LEDs - FW " + inf.version + " - " +
                    String(inf.latency_ms) + "ms";
    show_result(true, inf.name.c_str(), detail.c_str());
  } else {
    s_valid = false;
    set_add_enabled(false);
    String detail = host + " did not respond";
    show_result(false, "No WLED device found", detail.c_str());
  }
}

static void add_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (!s_valid) {
    return; // must pass Test Connection first
  }
  wled_store::save(s_host.c_str(), s_name.c_str(), s_leds);
  screen_manager::pop(); // back to the list, which rebuilds on load
}

static void back_cb(lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); }

lv_obj_t *wled_add_manual_create()
{
  create_screen_wled_add_manual();

  lv_obj_add_style(objects.wled_add_manual, &styles::style_screen, 0);

  s_valid = false;
  s_host = "";
  s_name = "";
  s_leds = 0;

  lv_obj_add_flag(objects.manual_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.manual_back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  lv_textarea_set_one_line(objects.manual_input, true);
  lv_keyboard_set_textarea(objects.manual_keyboard, objects.manual_input);
  // The keyboard's checkmark (READY) also runs Test Connection.
  lv_obj_add_event_cb(objects.manual_keyboard, test_cb, LV_EVENT_READY, NULL);

  lv_obj_add_flag(objects.manual_test_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.manual_test_btn, test_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_flag(objects.manual_add_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.manual_add_btn, add_cb, LV_EVENT_CLICKED, NULL);
  set_add_enabled(false); // until a successful test

  lv_obj_add_flag(objects.manual_result, LV_OBJ_FLAG_HIDDEN);

  return objects.wled_add_manual;
}
