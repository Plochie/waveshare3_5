#include "core/wled_client.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

namespace wled_client {

static constexpr uint16_t TIMEOUT_MS = 1500;

static bool wifi_ready()
{
  return WiFi.status() == WL_CONNECTED;
}

static String url_for(const char *host, const char *path)
{
  String u = "http://";
  u += host;
  u += path;
  return u;
}

// Issues a GET and parses the body into `doc` (filtered to `filter` to bound
// memory). Returns the HTTP latency in ms via `latency`, or false on failure.
static bool get_json(const char *host, const char *path, JsonDocument &doc,
                     const JsonDocument &filter, uint32_t *latency = nullptr)
{
  if (!wifi_ready()) return false;

  HTTPClient http;
  http.setConnectTimeout(TIMEOUT_MS);
  http.setTimeout(TIMEOUT_MS);
  if (!http.begin(url_for(host, path))) {
    return false;
  }

  uint32_t t0 = millis();
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  DeserializationError err = deserializeJson(doc, http.getStream(),
                                             DeserializationOption::Filter(filter));
  if (latency) *latency = millis() - t0;
  http.end();
  return !err;
}

static bool post_state(const char *host, const String &body)
{
  if (!wifi_ready()) return false;

  HTTPClient http;
  http.setConnectTimeout(TIMEOUT_MS);
  http.setTimeout(TIMEOUT_MS);
  if (!http.begin(url_for(host, "/json/state"))) {
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  http.end();
  return code == HTTP_CODE_OK;
}

bool get_state(const char *host, status &out)
{
  // Filter: only on / bri / seg[0].col[0].
  JsonDocument filter;
  filter["on"] = true;
  filter["bri"] = true;
  filter["seg"][0]["col"] = true;

  JsonDocument doc;
  if (!get_json(host, "/json/state", doc, filter)) {
    return false;
  }

  out.on = doc["on"] | false;
  out.bri = (uint8_t)(doc["bri"] | 0);
  JsonArrayConst col = doc["seg"][0]["col"][0];
  out.r = col.isNull() ? 0 : (uint8_t)(col[0] | 0);
  out.g = col.isNull() ? 0 : (uint8_t)(col[1] | 0);
  out.b = col.isNull() ? 0 : (uint8_t)(col[2] | 0);
  return true;
}

bool get_info(const char *host, info &out)
{
  JsonDocument filter;
  filter["name"] = true;
  filter["ver"] = true;
  filter["leds"]["count"] = true;

  JsonDocument doc;
  uint32_t latency = 0;
  if (!get_json(host, "/json/info", doc, filter, &latency)) {
    return false;
  }

  // A real WLED /json/info always carries a version string; treat its absence
  // as "not a WLED device".
  if (!doc["ver"].is<const char *>()) {
    return false;
  }
  out.name = (const char *)(doc["name"] | "WLED");
  out.version = (const char *)(doc["ver"] | "");
  out.leds = (uint16_t)(doc["leds"]["count"] | 0);
  out.latency_ms = latency;
  return true;
}

bool set_power(const char *host, bool on)
{
  return post_state(host, on ? "{\"on\":true}" : "{\"on\":false}");
}

bool set_brightness(const char *host, uint8_t bri)
{
  String body = "{\"bri\":";
  body += bri;
  body += "}";
  return post_state(host, body);
}

bool set_color(const char *host, uint8_t r, uint8_t g, uint8_t b)
{
  String body = "{\"seg\":[{\"col\":[[";
  body += r; body += ',';
  body += g; body += ',';
  body += b;
  body += "]]}]}";
  return post_state(host, body);
}

} // namespace wled_client
