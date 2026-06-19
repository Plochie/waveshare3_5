#pragma once

#include <lvgl.h>

// Creates the "Add WLED Device" screen: auto-discovers WLED devices on the
// local network via mDNS (background task + countdown), shows found/empty
// states, lets the user add discovered devices, and offers a manual-entry
// fallback.
lv_obj_t *wled_add_create();
