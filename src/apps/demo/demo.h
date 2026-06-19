#pragma once

#include <lvgl.h>

// Creates the demo screen (label + two switches). Used to validate the
// screen_manager / app_registry / launcher pipeline.
lv_obj_t *demo_create();
