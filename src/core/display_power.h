#pragma once

#include <stdint.h>

// Owns live screen brightness and auto-dim state. The settings UI and
// main.cpp go through this rather than touching settings_store/hal_display
// directly, so brightness/dim-timeout changes are applied and persisted in
// one place.
namespace display_power {

// Loads persisted settings, applies brightness via hal_display, and starts
// the auto-dim idle timer. Call once after the LVGL display is created.
void init();

// Clamped to 10-100. Applies immediately (unless currently dimmed) and
// persists via settings_store.
void set_brightness(uint8_t pct);

// One of 0 (never dim), 15, 30, 60, 120, 300 seconds. Persists via
// settings_store.
void set_dim_timeout(uint16_t sec);

uint8_t brightness();
uint16_t dim_timeout();

} // namespace display_power
