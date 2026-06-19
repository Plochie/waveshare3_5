#include "ui/styles.h"

namespace styles {

lv_style_t style_screen;
lv_style_t style_card;

void init()
{
  lv_style_init(&style_screen);
  lv_style_set_bg_color(&style_screen, bg_primary());
  lv_style_set_bg_opa(&style_screen, LV_OPA_COVER);

  lv_style_init(&style_card);
  lv_style_set_bg_color(&style_card, bg_surface());
  lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
  lv_style_set_border_color(&style_card, border());
  lv_style_set_border_width(&style_card, 1);
  lv_style_set_radius(&style_card, 8);
}

} // namespace styles
