#pragma once

#include <Arduino.h>
#include <stddef.h>

// Persists the list of known WLED devices (host + friendly name + LED count)
// to the SD card so they survive reboots. Plain text, local-storage only,
// following the same line-based convention as core/wifi_store.
namespace wled_store {

struct device {
  String name;   // friendly name (from WLED info, or host if unknown)
  String host;   // IP address or hostname (the unique key)
  uint16_t leds; // LED count reported by the device when it was added
};

// Most recently added device is kept first; oldest entries drop once this
// many are stored.
constexpr size_t MAX_DEVICES = 16;

// Loads up to `max_count` known devices (most-recent first) into `out`.
// Returns the number loaded (0 if no file or no SD card).
size_t load(device out[], size_t max_count);

// Adds `host` as the most-recently-used device (updating an existing entry
// for the same host), and persists immediately. No-op if SD isn't mounted.
void save(const char *host, const char *name, uint16_t leds);

// Forgets the device with the given host. No-op if absent or SD not mounted.
void remove(const char *host);

} // namespace wled_store
