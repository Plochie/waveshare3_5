#pragma once

#include <lvgl.h>

// Password-entry + connect screen for one chosen WiFi network. Built on the
// EEZ "wifi_connect" screen (src/ui/eez_export). Pushed from a wifi_manager
// card tap; pops back to wifi_manager once the join succeeds.
lv_obj_t *wifi_connect_create(const char *ssid);
