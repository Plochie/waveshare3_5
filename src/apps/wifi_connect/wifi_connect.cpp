#include "apps/wifi_connect/wifi_connect.h"

#include <Arduino.h>
// Same LDF note as wifi_manager: pull in the arduino-esp32 "Network" lib that
// WiFi depends on (WiFi declares no `depends`).
#include <Network.h>
#include <WiFi.h>
#include <string.h>

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "core/logging.h"
#include "core/wifi_store.h"

// Give up (and report failure) if the join hasn't completed in this long.
static constexpr uint32_t CONNECT_TIMEOUT_MS = 15000;

static char s_ssid[64];
static char s_password[64];
static lv_timer_t *s_poll_timer = nullptr;
static uint32_t s_connect_start = 0;
static bool s_connecting = false; // guards against a re-entrant WiFi.begin()

static void stop_poll()
{
  if (s_poll_timer) {
    lv_timer_delete(s_poll_timer);
    s_poll_timer = nullptr;
  }
}

static void set_status(const char *msg, lv_color_t color)
{
  lv_label_set_text(objects.connect_status, msg);
  lv_obj_set_style_text_color(objects.connect_status, color, 0);
}

// Stop the attempt cleanly: kill the poll timer and drop the (failed)
// association so the radio stops retrying the bad credentials.
static void finish_fail(const char *msg)
{
  stop_poll();
  s_connecting = false;
  WiFi.disconnect();
  LOG_W("wifi", "connect to '%s' failed: %s", s_ssid, msg);
  set_status(msg, styles::accent_red());
}

// Polls the join in progress; pops back to wifi_manager on success.
static void poll_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) {
    stop_poll();
    s_connecting = false;
    LOG_I("wifi", "connected to '%s' as %s", s_ssid,
          WiFi.localIP().toString().c_str());
    wifi_store::save(s_ssid, s_password);
    screen_manager::pop(); // wifi_manager's IP line updates via its own timer
    return;
  }
  if (st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL) {
    finish_fail("Connection failed - check password");
    return;
  }
  if (millis() - s_connect_start > CONNECT_TIMEOUT_MS) {
    finish_fail("Connection timed out");
  }
}

static void do_connect()
{
  if (s_connecting) {
    return; // ignore double triggers (Connect button + keyboard checkmark)
  }
  const char *pw = lv_textarea_get_text(objects.pw_input);
  strncpy(s_password, pw ? pw : "", sizeof(s_password) - 1);
  s_password[sizeof(s_password) - 1] = '\0';

  set_status("Connecting...", styles::accent_amber());
  s_connecting = true;

  // We drive the initial attempt ourselves: clear any in-flight attempt
  // before issuing a fresh begin() (the source of the "sta is connecting,
  // cannot set config" errors), but let the driver auto-reconnect afterwards
  // so a transient drop recovers on its own.
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(s_ssid, s_password);
  s_connect_start = millis();

  if (!s_poll_timer) {
    s_poll_timer = lv_timer_create(poll_cb, 500, NULL);
  }
}

static void connect_cb(lv_event_t *e) { LV_UNUSED(e); do_connect(); }

// The keyboard's checkmark (READY) also triggers the connect attempt.
static void kb_ready_cb(lv_event_t *e) { LV_UNUSED(e); do_connect(); }

static void back_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  stop_poll();
  s_connecting = false;
}

lv_obj_t *wifi_connect_create(const char *ssid)
{
  strncpy(s_ssid, ssid ? ssid : "", sizeof(s_ssid) - 1);
  s_ssid[sizeof(s_ssid) - 1] = '\0';

  create_screen_wifi_connect();
  lv_obj_add_style(objects.wifi_connect, &styles::style_screen, 0);

  lv_label_set_text_fmt(objects.connect_ssid, "%s",
                        s_ssid[0] ? s_ssid : "(hidden network)");

  // Route the keyboard into the password field; keep it masked + single line.
  lv_textarea_set_password_mode(objects.pw_input, true);
  lv_textarea_set_one_line(objects.pw_input, true);
  lv_keyboard_set_textarea(objects.pw_keyboard, objects.pw_input);
  lv_obj_add_event_cb(objects.pw_keyboard, kb_ready_cb, LV_EVENT_READY, NULL);

  // connect_btn / connect_back_btn are EEZ labels; make them tappable.
  lv_obj_add_flag(objects.connect_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.connect_btn, connect_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(objects.connect_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.connect_back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_event_cb(objects.wifi_connect, screen_delete_cb, LV_EVENT_DELETE,
                      NULL);

  set_status("", styles::text_muted());
  return objects.wifi_connect;
}
