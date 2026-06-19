#pragma once

#include <lvgl.h>

// "Logs" app: a scrolling on-screen view of the in-RAM log ring buffer
// (see core/logging.h), color-coded by level and auto-scrolling to newest.
lv_obj_t *logs_create();
