#pragma once

#include <Arduino.h>
#include <utility>
#include <vector>

// Parses the Stream Deck config (/deck/config.json on SD) into an in-memory
// model. Schema: docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md.
namespace deck_config {

enum class step_type {
  unknown,
  http_request,
  ha_service,
  ha_webhook,
  mqtt_publish,   // parsed, not executed in phase 1
  hotkey,         // host steps: parsed, skipped in phase 1
  type_text,
  launch_app,
  run_command,
  media_key,
  delay,
};

struct step {
  step_type type = step_type::unknown;

  // http_request
  String method;   // "GET" | "POST" | "PUT" | "DELETE"
  String url;
  std::vector<std::pair<String, String>> headers;
  String body;

  // ha_service
  String domain;
  String service;
  String data_json;   // serialized "data" object, sent as the POST body

  // ha_webhook
  String webhook_id;

  // mqtt_publish (phase 4)
  String topic;
  String payload;

  // delay
  uint32_t delay_ms = 0;
};

struct button {
  int pos = -1;
  String label;
  String icon;       // filename under /deck/icons/, or empty
  String color;      // "#RRGGBB" or empty
  String open_page;  // folder target page id, or empty
  std::vector<step> steps;
};

struct page {
  String id;
  String title;
  std::vector<button> buttons;
};

struct config {
  int version = 0;
  int cols = 3;
  int rows = 4;
  String ha_base_url;
  String ha_token;
  String agent_token;   // pairing secret, matched against the WS hello/auth
  std::vector<page> pages;
};

// Parses `json` into `out`. Returns false on JSON syntax error. Unknown step
// types parse as step_type::unknown (skipped later, not an error).
bool parse(const String &json, config &out);

// Reads /deck/config.json from SD and parses it. Returns false if no SD card,
// missing file, or parse error.
bool load(config &out);

// Returns the page with the given id, or nullptr.
const page *find_page(const config &c, const String &id);

// Parses an embedded sample config and logs the resulting structure (bring-up
// aid, mirrors sdcard::self_test).
void self_test();

} // namespace deck_config
