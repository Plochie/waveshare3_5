#pragma once

#include <Arduino.h>
#include <stddef.h>

// Persists known WiFi networks (SSID + password) to the SD card so the
// device can auto-connect on boot. Plain text, local-storage only.
namespace wifi_store {

struct credential {
  String ssid;
  String password;
};

// Most recently used network is kept first; oldest entries are dropped once
// this many are stored.
constexpr size_t MAX_NETWORKS = 5;

// Loads up to `max_count` known networks (most-recent first) into `out`.
// Returns the number loaded (0 if no file or no SD card).
size_t load(credential out[], size_t max_count);

// Adds `ssid`/`password` as the most-recently-used network, updating an
// existing entry for the same SSID if present, and persists immediately.
// No-op if the SD card isn't mounted.
void save(const char *ssid, const char *password);

} // namespace wifi_store
