#pragma once

#include <lvgl.h>

// Creates the I2C/SPI Scanner screen (src/ui/eez_export). Scans the shared
// I2C bus (Wire, addresses 0x03-0x77) on demand and renders an address-map
// grid alongside a list of responding devices.
lv_obj_t *i2c_spi_scanner_create();
