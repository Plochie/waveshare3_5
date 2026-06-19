#pragma once

#include <lvgl.h>
#include <stdint.h>

// Creates the WLED device-control screen for one device: power, hue +
// saturation + brightness sliders, a live color preview, and a quick-color
// palette. Reads initial state from the device and pushes changes back via
// wled_client (debounced).
lv_obj_t *wled_detail_create(const char *host, const char *name, uint16_t leds);
