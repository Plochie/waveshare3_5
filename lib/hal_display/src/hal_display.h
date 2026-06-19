#pragma once

#include <Arduino_GFX_Library.h>

// Resets the panel via the TCA9554 IO-expander and initializes the
// AXS15231B QSPI display. Returns the Arduino_GFX instance used for
// flush operations, or nullptr if gfx->begin() fails.
Arduino_GFX *hal_display_init();

// Configures the backlight pin (GFX_BL) for PWM brightness control via ledc.
// Call once at boot, after hal_display_init().
void hal_display_backlight_init();

// Sets backlight brightness, 0-100 (%). 0 = fully off (no light).
void hal_display_set_brightness(uint8_t pct);
