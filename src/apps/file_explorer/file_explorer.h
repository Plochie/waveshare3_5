#pragma once

#include <lvgl.h>

// Lists the SD card directory at `path` (absolute, e.g. "/" or "/logs").
// Tapping a subdirectory pushes another instance of this screen for that
// path; tapping a text file pushes file_viewer. Suitable for
// screen_manager::push().
lv_obj_t *file_explorer_create(const char *path = "/");
