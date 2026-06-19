#pragma once

#include <Arduino.h>
#include <stdint.h>

// Minimal client for the WLED JSON HTTP API (https://kno.wled.ge/interfaces/json-api/).
// All calls are synchronous and short-timeout; every call returns false on any
// error (no WiFi, unreachable host, bad JSON) rather than throwing.
namespace wled_client {

struct status {
  bool on;
  uint8_t bri;        // 0-255
  uint8_t r, g, b;    // primary segment color
};

struct info {
  String name;        // device name (info.name)
  uint16_t leds;      // info.leds.count
  String version;     // info.ver
  uint32_t latency_ms; // round-trip time of the probe request
};

// GET http://<host>/json/state  -> on / bri / primary color.
bool get_state(const char *host, status &out);

// GET http://<host>/json/info  -> name / led count / version (+ latency).
// Used both to refresh metadata and as the manual-add "Test Connection" probe;
// a successful parse means the host is a real WLED device.
bool get_info(const char *host, info &out);

// POST http://<host>/json/state with the given field.
bool set_power(const char *host, bool on);
bool set_brightness(const char *host, uint8_t bri);
bool set_color(const char *host, uint8_t r, uint8_t g, uint8_t b);

} // namespace wled_client
