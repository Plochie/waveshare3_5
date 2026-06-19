#pragma once

#include <lvgl.h>

// Pushes a read-only text viewer for the file at `path` (absolute, SD-rooted,
// e.g. "/logs/run.txt"). Suitable for screen_manager::push().
lv_obj_t *file_viewer_create(const char *path);
