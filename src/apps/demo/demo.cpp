#include "apps/demo/demo.h"

lv_obj_t *demo_create()
{
  lv_obj_t *scr = lv_obj_create(NULL);

  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text_fmt(label, "Hello Arduino, I'm LVGL! (V%d.%d.%d)",
                         lv_version_major(), lv_version_minor(), lv_version_patch());
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *sw = lv_switch_create(scr);
  lv_obj_align(sw, LV_ALIGN_TOP_MID, 0, 50);

  sw = lv_switch_create(scr);
  lv_obj_align(sw, LV_ALIGN_BOTTOM_MID, 0, -50);

  return scr;
}
