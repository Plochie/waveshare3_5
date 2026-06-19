#include "core/display_power.h"

#include <lvgl.h>

#include "core/settings_store.h"
#include "hal_display.h"

namespace display_power {

static settings_store::settings_t s_settings;
static bool s_dimmed = false;

// Polls LVGL's per-display idle time once a second. Backlight goes fully off
// after dim_timeout seconds of no touch, and is restored to the saved
// brightness on the next touch (LVGL resets inactive time automatically).
static void check_idle_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  lv_display_t *disp = lv_display_get_default();
  uint32_t idle_ms = lv_display_get_inactive_time(disp);

  if (s_settings.dim_timeout != 0 &&
      idle_ms >= (uint32_t)s_settings.dim_timeout * 1000) {
    if (!s_dimmed) {
      hal_display_set_brightness(0);
      s_dimmed = true;
    }
  } else if (s_dimmed) {
    hal_display_set_brightness(s_settings.brightness);
    s_dimmed = false;
  }
}

void init()
{
  s_settings = settings_store::load();
  hal_display_set_brightness(s_settings.brightness);
  lv_timer_create(check_idle_cb, 1000, NULL);
}

void set_brightness(uint8_t pct)
{
  if (pct < 10) {
    pct = 10;
  } else if (pct > 100) {
    pct = 100;
  }
  s_settings.brightness = pct;
  if (!s_dimmed) {
    hal_display_set_brightness(pct);
  }
  settings_store::save(s_settings);
}

void set_dim_timeout(uint16_t sec)
{
  s_settings.dim_timeout = sec;
  settings_store::save(s_settings);
}

uint8_t brightness()
{
  return s_settings.brightness;
}

uint16_t dim_timeout()
{
  return s_settings.dim_timeout;
}

} // namespace display_power
