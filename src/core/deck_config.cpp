#include "core/deck_config.h"

#include <ArduinoJson.h>

#include "core/logging.h"
#include "hal_sdcard.h"

namespace deck_config {

static constexpr const char *PATH = "/deck/config.json";

static step_type type_from_string(const char *s)
{
  if (!s) return step_type::unknown;
  if (!strcmp(s, "http_request")) return step_type::http_request;
  if (!strcmp(s, "ha_service"))   return step_type::ha_service;
  if (!strcmp(s, "ha_webhook"))   return step_type::ha_webhook;
  if (!strcmp(s, "mqtt_publish")) return step_type::mqtt_publish;
  if (!strcmp(s, "hotkey"))       return step_type::hotkey;
  if (!strcmp(s, "type_text"))    return step_type::type_text;
  if (!strcmp(s, "launch_app"))   return step_type::launch_app;
  if (!strcmp(s, "run_command"))  return step_type::run_command;
  if (!strcmp(s, "media_key"))    return step_type::media_key;
  if (!strcmp(s, "delay"))        return step_type::delay;
  return step_type::unknown;
}

static void parse_step(JsonObjectConst o, step &st)
{
  st.type = type_from_string(o["type"] | "");
  switch (st.type) {
    case step_type::http_request:
      st.method = (const char *)(o["method"] | "GET");
      st.url = (const char *)(o["url"] | "");
      st.body = (const char *)(o["body"] | "");
      for (JsonPairConst h : o["headers"].as<JsonObjectConst>()) {
        st.headers.emplace_back(String(h.key().c_str()),
                                String((const char *)(h.value() | "")));
      }
      break;
    case step_type::ha_service:
      st.domain = (const char *)(o["domain"] | "");
      st.service = (const char *)(o["service"] | "");
      if (!o["data"].isNull()) {
        serializeJson(o["data"], st.data_json);
      }
      break;
    case step_type::ha_webhook:
      st.webhook_id = (const char *)(o["id"] | "");
      break;
    case step_type::mqtt_publish:
      st.topic = (const char *)(o["topic"] | "");
      st.payload = (const char *)(o["payload"] | "");
      break;
    case step_type::delay:
      st.delay_ms = (uint32_t)(o["ms"] | 0);
      break;
    default:
      break; // host steps + unknown: nothing to extract in phase 1
  }
}

bool parse(const String &json, config &out)
{
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    LOG_E("deck", "config parse failed: %s", err.c_str());
    return false;
  }

  out = config{}; // reset to defaults
  out.version = doc["version"] | 0;
  out.cols = doc["grid"]["cols"] | 3;
  out.rows = doc["grid"]["rows"] | 4;
  out.ha_base_url = (const char *)(doc["ha"]["base_url"] | "");
  out.ha_token = (const char *)(doc["ha"]["token"] | "");
  out.agent_token = (const char *)(doc["agent"]["token"] | "");

  for (JsonObjectConst p : doc["pages"].as<JsonArrayConst>()) {
    page pg;
    pg.id = (const char *)(p["id"] | "");
    pg.title = (const char *)(p["title"] | "");
    for (JsonObjectConst b : p["buttons"].as<JsonArrayConst>()) {
      button btn;
      btn.pos = b["pos"] | -1;
      btn.label = (const char *)(b["label"] | "");
      btn.icon = (const char *)(b["icon"] | "");
      btn.color = (const char *)(b["color"] | "");
      btn.open_page = (const char *)(b["open_page"] | "");
      for (JsonObjectConst s : b["steps"].as<JsonArrayConst>()) {
        step st;
        parse_step(s, st);
        btn.steps.push_back(st);
      }
      pg.buttons.push_back(btn);
    }
    out.pages.push_back(pg);
  }
  return true;
}

bool load(config &out)
{
  if (!sdcard::is_mounted()) {
    LOG_W("deck", "no SD card; cannot load %s", PATH);
    return false;
  }
  if (!sdcard::exists(PATH)) {
    LOG_W("deck", "%s not found", PATH);
    return false;
  }
  String json = sdcard::read_string(PATH);
  return parse(json, out);
}

const page *find_page(const config &c, const String &id)
{
  for (const auto &p : c.pages) {
    if (p.id == id) return &p;
  }
  return nullptr;
}

void self_test()
{
  static const char *SAMPLE =
      "{\"version\":1,\"grid\":{\"cols\":3,\"rows\":4},"
      "\"ha\":{\"base_url\":\"http://ha.local:8123\",\"token\":\"t\"},"
      "\"pages\":[{\"id\":\"home\",\"title\":\"Home\",\"buttons\":["
      "{\"pos\":0,\"label\":\"Hi\",\"steps\":[{\"type\":\"ha_webhook\",\"id\":\"x\"},"
      "{\"type\":\"delay\",\"ms\":500}]},"
      "{\"pos\":1,\"label\":\"Work\",\"open_page\":\"work\"}]}]}";
  config c;
  if (!parse(String(SAMPLE), c)) {
    LOG_E("deck", "self_test: parse failed");
    return;
  }
  LOG_I("deck", "self_test: ver=%d grid=%dx%d pages=%u", c.version, c.cols,
        c.rows, (unsigned)c.pages.size());
  const page *home = find_page(c, "home");
  if (home) {
    LOG_I("deck", "self_test: home buttons=%u btn0 steps=%u btn1 open_page=%s",
          (unsigned)home->buttons.size(),
          (unsigned)home->buttons[0].steps.size(),
          home->buttons[1].open_page.c_str());
  }
}

} // namespace deck_config
