#include "apps/settings/settings.h"

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "core/display_power.h"

static void back_event_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

// Brightness slider: live-applies + persists via display_power, and updates
// the "%" label.
static void brightness_changed_cb(lv_event_t *e)
{
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int32_t value = lv_slider_get_value(slider);
  display_power::set_brightness((uint8_t)value);
  lv_label_set_text_fmt(objects.brightness_value, "%d%%", (int)value);
}

// Cycles dim_value_btn through the preset auto-dim timeouts and updates the
// label + display_power state.
static const uint16_t kDimOptions[] = {0, 15, 30, 60, 120, 300};
static const char *kDimLabels[] = {"Off", "15s", "30s", "1m", "2m", "5m"};
static constexpr size_t kDimOptionCount = sizeof(kDimOptions) / sizeof(kDimOptions[0]);

static size_t dim_index_for(uint16_t seconds)
{
  for (size_t i = 0; i < kDimOptionCount; i++) {
    if (kDimOptions[i] == seconds) {
      return i;
    }
  }
  return 0;
}

static void dim_value_clicked_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  size_t next = (dim_index_for(display_power::dim_timeout()) + 1) % kDimOptionCount;
  display_power::set_dim_timeout(kDimOptions[next]);
  lv_label_set_text(objects.dim_value_btn, kDimLabels[next]);
}

lv_obj_t *settings_create()
{
  create_screen_settings();

  lv_obj_add_style(objects.settings, &styles::style_screen, 0);

  lv_obj_add_flag(objects.settings_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.settings_back_btn, back_event_cb, LV_EVENT_CLICKED, NULL);

  uint8_t brightness = display_power::brightness();
  lv_slider_set_value(objects.brightness_slider, brightness, LV_ANIM_OFF);
  lv_label_set_text_fmt(objects.brightness_value, "%d%%", (int)brightness);
  lv_obj_add_event_cb(objects.brightness_slider, brightness_changed_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_label_set_text(objects.dim_value_btn,
                     kDimLabels[dim_index_for(display_power::dim_timeout())]);
  lv_obj_add_flag(objects.dim_value_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.dim_value_btn, dim_value_clicked_cb, LV_EVENT_CLICKED, NULL);

  return objects.settings;
}
