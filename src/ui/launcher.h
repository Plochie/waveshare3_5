#pragma once

#include <lvgl.h>

namespace launcher {

// Creates the launcher screen with one button per app_registry entry.
// Tapping a button pushes that app's screen via screen_manager.
lv_obj_t *create();

} // namespace launcher
