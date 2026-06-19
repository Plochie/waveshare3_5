#pragma once

#include <lvgl.h>

// Creates the EEZ Studio "main" screen (src/ui/eez_export). Validates the
// EEZ export -> app_registry -> screen_manager pipeline.
lv_obj_t *eez_demo_create();
