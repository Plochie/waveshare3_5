#pragma once

#include <lvgl.h>

// Creates the "Add Manually" screen: enter a WLED device IP/hostname, Test
// Connection (probes via wled_client::get_info), then Add Device (persists to
// wled_store and returns to the device list).
lv_obj_t *wled_add_manual_create();
