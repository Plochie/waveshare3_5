#include "apps/wled/wled_add.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include "apps/wled/wled_add_manual.h"
#include "core/screen_manager.h"
#include "core/wled_client.h"
#include "core/wled_store.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"

static constexpr int MAX_FOUND = 12;
static constexpr uint32_t SCAN_BUDGET_MS = 4500;

struct found_dev {
  String name;
  String host;
  uint16_t leds;
  String ver;
  bool already;
};

// Shared between the scan task (writer) and the poll timer (reader). The task
// touches only this plain data, never LVGL objects.
static found_dev s_found[MAX_FOUND];
static volatile int s_found_count = 0;
static volatile bool s_done = false;
static volatile bool s_scanning = false;
static uint32_t s_start_ms = 0;
static bool s_mdns_started = false;

static lv_timer_t *s_poll = nullptr;

enum section { SEC_SCANNING, SEC_RESULTS, SEC_EMPTY };

// ---- scan task (runs off the LVGL thread) ----

static bool host_known(const wled_store::device *known, size_t kn,
                       const String &host, const String &name)
{
  for (size_t i = 0; i < kn; i++) {
    if (known[i].host == host || known[i].name == name) return true;
  }
  return false;
}

static void collect(const char *service, found_dev *local, int &n, uint32_t start)
{
  int cnt = MDNS.queryService(service, "tcp");
  for (int i = 0; i < cnt && n < MAX_FOUND; i++) {
    if (millis() - start > SCAN_BUDGET_MS) break;
    String host = MDNS.address(i).toString();
    String hostname = MDNS.hostname(i);
    if (host == "0.0.0.0" || host.length() == 0) continue;
    // For the generic _http._tcp pass, only adopt wled-* hostnames.
    if (strcmp(service, "http") == 0 && !hostname.startsWith("wled")) continue;
    // De-dupe by host.
    bool dup = false;
    for (int j = 0; j < n; j++) {
      if (local[j].host == host) { dup = true; break; }
    }
    if (dup) continue;

    found_dev d;
    d.host = host;
    d.name = hostname.length() ? hostname : host;
    d.leds = 0;
    d.already = false;
    wled_client::info inf;
    if (wled_client::get_info(host.c_str(), inf)) {
      d.name = inf.name;
      d.leds = inf.leds;
      d.ver = inf.version;
    } else if (strcmp(service, "http") == 0) {
      continue; // unconfirmed http device — skip
    }
    local[n++] = d;
  }
}

static void scan_task(void *arg)
{
  LV_UNUSED(arg);
  found_dev local[MAX_FOUND];
  int n = 0;
  uint32_t start = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!s_mdns_started) {
      MDNS.begin("wled-scanner");
      s_mdns_started = true;
    }
    collect("wled", local, n, start);
    if (n < MAX_FOUND) collect("http", local, n, start);

    wled_store::device known[wled_store::MAX_DEVICES];
    size_t kn = wled_store::load(known, wled_store::MAX_DEVICES);
    for (int i = 0; i < n; i++) {
      local[i].already = host_known(known, kn, local[i].host, local[i].name);
    }
  }

  for (int i = 0; i < n; i++) s_found[i] = local[i];
  s_found_count = n;
  s_done = true;
  s_scanning = false;
  vTaskDelete(NULL);
}

static void start_scan()
{
  if (s_scanning) return; // a task is still running (e.g. quick re-entry)
  s_done = false;
  s_found_count = 0;
  s_scanning = true;
  s_start_ms = millis();
  xTaskCreate(scan_task, "wled_scan", 10240, NULL, 1, NULL);
}

// ---- UI ----

static void show_section(section sec)
{
  lv_obj_add_flag(objects.scan_section, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(objects.results_section, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(objects.empty_section, LV_OBJ_FLAG_HIDDEN);
  bool footer = (sec != SEC_SCANNING);
  if (footer) {
    lv_obj_clear_flag(objects.scan_again_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(objects.add_manual_btn, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(objects.scan_again_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.add_manual_btn, LV_OBJ_FLAG_HIDDEN);
  }
  switch (sec) {
    case SEC_SCANNING: lv_obj_clear_flag(objects.scan_section, LV_OBJ_FLAG_HIDDEN); break;
    case SEC_RESULTS:  lv_obj_clear_flag(objects.results_section, LV_OBJ_FLAG_HIDDEN); break;
    case SEC_EMPTY:    lv_obj_clear_flag(objects.empty_section, LV_OBJ_FLAG_HIDDEN); break;
  }
}

static void add_switch_cb(lv_event_t *e)
{
  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  if (idx < 0 || idx >= s_found_count) return;
  lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
  bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);
  if (on) {
    wled_store::save(s_found[idx].host.c_str(), s_found[idx].name.c_str(), s_found[idx].leds);
    s_found[idx].already = true;
  } else {
    wled_store::remove(s_found[idx].host.c_str());
    s_found[idx].already = false;
  }
}

static void make_result_row(int idx)
{
  const found_dev &d = s_found[idx];
  lv_obj_t *card = lv_obj_create(objects.results_list);
  lv_obj_set_size(card, 304, 56);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_style_bg_color(card, styles::bg_card(), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(card, 8, 0);
  lv_obj_set_style_border_color(card, styles::border(), 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_pad_all(card, 0, 0);

  lv_obj_t *name = lv_label_create(card);
  lv_obj_set_pos(name, 12, 9);
  lv_obj_set_width(name, 200);
  lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(name, styles::text_primary(), 0);
  lv_label_set_text(name, d.name.c_str());

  lv_obj_t *detail = lv_label_create(card);
  lv_obj_set_pos(detail, 12, 31);
  lv_obj_set_width(detail, 230);
  lv_label_set_long_mode(detail, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(detail, styles::text_muted(), 0);
  if (d.leds) {
    lv_label_set_text_fmt(detail, "%s - %u LEDs - FW %s", d.host.c_str(),
                          (unsigned)d.leds, d.ver.c_str());
  } else {
    lv_label_set_text(detail, d.host.c_str());
  }

  lv_obj_t *sw = lv_switch_create(card);
  lv_obj_set_size(sw, 44, 22);
  lv_obj_set_style_bg_color(sw, styles::accent_green(), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -12, 0);
  if (d.already) lv_obj_add_state(sw, LV_STATE_CHECKED);
  lv_obj_add_event_cb(sw, add_switch_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)idx);
}

static void build_results()
{
  lv_obj_clean(objects.results_list);
  lv_obj_set_flex_flow(objects.results_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(objects.results_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(objects.results_list, 8, 0);
  for (int i = 0; i < s_found_count; i++) make_result_row(i);
  lv_label_set_text_fmt(objects.results_banner, "Found %d device%s on network",
                        s_found_count, s_found_count == 1 ? "" : "s");
}

static void poll_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  uint32_t el = millis() - s_start_ms;
  float secs = el / 1000.0f;
  if (secs > 5.0f) secs = 5.0f;
  lv_label_set_text_fmt(objects.scan_timer, "%.1fs / 5s", secs);

  if (!s_done) return;

  lv_timer_delete(s_poll);
  s_poll = nullptr;
  if (s_found_count > 0) {
    build_results();
    show_section(SEC_RESULTS);
  } else {
    show_section(SEC_EMPTY);
  }
}

static void begin_scan_ui()
{
  show_section(SEC_SCANNING);
  start_scan();
  if (!s_poll) {
    s_poll = lv_timer_create(poll_cb, 100, NULL);
  }
}

static void back_cb(lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); }
static void manual_cb(lv_event_t *e) { LV_UNUSED(e); screen_manager::push(wled_add_manual_create()); }
static void again_cb(lv_event_t *e) { LV_UNUSED(e); begin_scan_ui(); }

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_poll) {
    lv_timer_delete(s_poll);
    s_poll = nullptr;
  }
  // The scan task (if any) finishes on its own and only writes plain data.
}

lv_obj_t *wled_add_create()
{
  create_screen_wled_add();
  lv_obj_add_style(objects.wled_add, &styles::style_screen, 0);
  lv_obj_add_event_cb(objects.wled_add, screen_delete_cb, LV_EVENT_DELETE, NULL);

  lv_obj_add_flag(objects.add_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.add_back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  // Spinner in the scanning section.
  lv_obj_t *sp = lv_spinner_create(objects.scan_section);
  lv_spinner_set_anim_params(sp, 1000, 60);
  lv_obj_set_size(sp, 56, 56);
  lv_obj_set_pos(sp, 132, 120);
  lv_obj_set_style_arc_color(sp, styles::border(), LV_PART_MAIN);
  lv_obj_set_style_arc_color(sp, styles::accent_green(), LV_PART_INDICATOR);

  lv_obj_add_flag(objects.scan_skip_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.scan_skip_btn, manual_cb, LV_EVENT_CLICKED, NULL);

  // Red ✗ mark above the empty-state title.
  lv_obj_t *x = lv_label_create(objects.empty_section);
  lv_label_set_text(x, LV_SYMBOL_CLOSE);
  lv_obj_set_style_text_color(x, styles::accent_red(), 0);
  lv_obj_set_style_text_font(x, &lv_font_montserrat_24, 0);
  lv_obj_set_width(x, 320);
  lv_obj_set_style_text_align(x, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(x, 0, 78);

  lv_obj_add_flag(objects.scan_again_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.scan_again_btn, again_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(objects.add_manual_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.add_manual_btn, manual_cb, LV_EVENT_CLICKED, NULL);

  begin_scan_ui();

  return objects.wled_add;
}
