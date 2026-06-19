#pragma once

#include <stdint.h>

// Small persisted key=value settings file on the SD card (/settings.txt),
// following the same line-based convention as core/wifi_store.
namespace settings_store {

struct settings_t {
  uint8_t brightness;    // 10-100 (%)
  uint16_t dim_timeout;  // seconds, 0 = never dim
};

// Returns defaults (brightness=100, dim_timeout=30) if /settings.txt is
// missing or no SD card is mounted.
settings_t load();

// No-op if no SD card is mounted.
void save(const settings_t &s);

} // namespace settings_store
