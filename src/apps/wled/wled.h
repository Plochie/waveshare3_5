#pragma once

#include <lvgl.h>

// Creates the WLED Lights screen (the app root): a list of saved WLED devices
// with inline power toggles and a live brightness/color readout, plus an
// "All Lights" master switch and a "+ Add" entry into the add-device flow.
lv_obj_t *wled_create();
