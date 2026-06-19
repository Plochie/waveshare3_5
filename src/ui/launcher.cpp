#include "ui/launcher.h"

#include "ui/styles.h"
#include "core/screen_manager.h"
#include "apps/app_registry.h"

namespace launcher {

static void open_app_event_cb(lv_event_t *e)
{
  auto *app = static_cast<const app_descriptor_t *>(lv_event_get_user_data(e));
  screen_manager::push(app->create());
}

lv_obj_t *create()
{
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_add_style(scr, &styles::style_screen, 0);
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  size_t count;
  const app_descriptor_t *apps = app_registry::all(count);

  for (size_t i = 0; i < count; i++)
  {
    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_add_event_cb(btn, open_app_event_cb, LV_EVENT_CLICKED,
                         const_cast<app_descriptor_t *>(&apps[i]));

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text_fmt(label, "Open %s", apps[i].name);
  }

  return scr;
}

} // namespace launcher
