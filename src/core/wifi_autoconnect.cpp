#include "core/wifi_autoconnect.h"

#include <Network.h>
#include <WiFi.h>
#include <lvgl.h>

#include "core/logging.h"
#include "core/wifi_store.h"

namespace wifi_autoconnect {

static constexpr uint32_t ATTEMPT_TIMEOUT_MS = 8000;

static wifi_store::credential s_creds[wifi_store::MAX_NETWORKS];
static size_t s_count = 0;
static size_t s_index = 0;
static uint32_t s_attempt_start = 0;
static lv_timer_t *s_timer = nullptr;

static void stop()
{
  if (s_timer) {
    lv_timer_delete(s_timer);
    s_timer = nullptr;
  }
}

static void try_current()
{
  LOG_I("wifi", "auto-connect attempt %u/%u: '%s'", (unsigned)(s_index + 1),
        (unsigned)s_count, s_creds[s_index].ssid.c_str());
  WiFi.disconnect();
  WiFi.begin(s_creds[s_index].ssid.c_str(), s_creds[s_index].password.c_str());
  s_attempt_start = millis();
}

static void poll_cb(lv_timer_t *t)
{
  LV_UNUSED(t);

  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) {
    LOG_I("wifi", "auto-connected to '%s' as %s",
          s_creds[s_index].ssid.c_str(), WiFi.localIP().toString().c_str());
    stop();
    return;
  }

  bool failed = (st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL);
  bool timed_out = (millis() - s_attempt_start) > ATTEMPT_TIMEOUT_MS;
  if (!failed && !timed_out) {
    return;
  }

  WiFi.disconnect();
  s_index++;
  if (s_index >= s_count) {
    LOG_W("wifi", "auto-connect: no known network reachable");
    stop();
    return;
  }
  try_current();
}

void start()
{
  s_count = wifi_store::load(s_creds, wifi_store::MAX_NETWORKS);
  if (s_count == 0) {
    LOG_I("wifi", "auto-connect: no known networks saved");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  s_index = 0;
  try_current();
  s_timer = lv_timer_create(poll_cb, 500, NULL);
}

} // namespace wifi_autoconnect
