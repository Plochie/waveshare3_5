#include "core/deck_executor.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "core/deck_client.h"
#include "core/logging.h"

namespace deck_executor {

static constexpr uint16_t HTTP_TIMEOUT_MS = 3000;
static constexpr uint32_t HOST_STEP_TIMEOUT_MS = 3000; // protocol §3.4/§4

// Heap payload handed to the worker task (owns its own copies).
struct job {
  std::vector<deck_config::step> steps;
  String ha_base_url;
  String ha_token;
};

static volatile bool s_busy = false;
static volatile bool s_active = false;
static volatile bool s_ok = false;
static char s_msg[64] = {0};

static void set_msg(const char *m)
{
  strncpy(s_msg, m, sizeof(s_msg) - 1);
  s_msg[sizeof(s_msg) - 1] = '\0';
}

static bool do_http(const String &method, const String &url,
                    const std::vector<std::pair<String, String>> &headers,
                    const String &body, const String &auth_bearer)
{
  if (WiFi.status() != WL_CONNECTED) {
    set_msg("WiFi not connected");
    return false;
  }
  HTTPClient http;
  http.setConnectTimeout(HTTP_TIMEOUT_MS);
  http.setTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(url)) {
    set_msg("bad URL");
    return false;
  }
  for (const auto &h : headers) {
    http.addHeader(h.first, h.second);
  }
  if (auth_bearer.length()) {
    http.addHeader("Authorization", "Bearer " + auth_bearer);
  }
  int code;
  if (method == "GET") {
    code = http.GET();
  } else if (method == "PUT") {
    code = http.PUT((uint8_t *)body.c_str(), body.length());
  } else if (method == "DELETE") {
    code = http.sendRequest("DELETE", (uint8_t *)body.c_str(), body.length());
  } else { // POST and default
    code = http.POST(body);
  }
  http.end();
  bool ok = (code >= 200 && code < 300);
  LOG_I("deck", "%s %s -> %d", method.c_str(), url.c_str(), code);
  if (!ok) {
    char b[64];
    snprintf(b, sizeof(b), "HTTP %d", code);
    set_msg(b);
  }
  return ok;
}

// Builds the protocol §3 step JSON for a host step (the wire shape, not
// deck_config::step's parsed-field layout) to send as `exec`'s "step".
static String host_step_json(const deck_config::step &st)
{
  using deck_config::step_type;
  JsonDocument doc;
  switch (st.type) {
    case step_type::hotkey: {
      doc["type"] = "hotkey";
      JsonArray keys = doc["keys"].to<JsonArray>();
      for (const String &k : st.keys) keys.add(k);
      break;
    }
    case step_type::type_text:
      doc["type"] = "type_text";
      doc["text"] = st.text;
      break;
    case step_type::launch_app:
      doc["type"] = "launch_app";
      doc["target"] = st.target;
      break;
    case step_type::run_command:
      doc["type"] = "run_command";
      doc["command"] = st.command;
      break;
    case step_type::media_key:
      doc["type"] = "media_key";
      doc["key"] = st.media_key;
      break;
    default:
      break;
  }
  String out;
  serializeJson(doc, out);
  return out;
}

// Sends a host step to the desktop agent over the exec/result WS exchange
// and blocks (this runs on the executor's own worker task) for the result.
static bool run_host_step(const deck_config::step &st)
{
  String err;
  bool ok = deck_client::exec_host_step(host_step_json(st), HOST_STEP_TIMEOUT_MS, err);
  if (!ok) {
    LOG_W("deck", "host step failed: %s", err.c_str());
    set_msg(err.length() ? err.c_str() : "host step failed");
  }
  return ok;
}

static bool run_step(const deck_config::step &st, const job &j)
{
  using deck_config::step_type;
  switch (st.type) {
    case step_type::http_request: {
      std::vector<std::pair<String, String>> hdrs = st.headers;
      String body = st.body;
      if (st.method != "GET" && body.length()) {
        bool has_ct = false;
        for (auto &h : hdrs) {
          if (h.first.equalsIgnoreCase("Content-Type")) has_ct = true;
        }
        if (!has_ct) hdrs.emplace_back("Content-Type", "application/json");
      }
      return do_http(st.method.length() ? st.method : "GET", st.url, hdrs, body, "");
    }
    case step_type::ha_service: {
      if (j.ha_base_url.isEmpty()) { set_msg("no HA base_url"); return false; }
      String url = j.ha_base_url + "/api/services/" + st.domain + "/" + st.service;
      std::vector<std::pair<String, String>> hdrs;
      hdrs.emplace_back("Content-Type", "application/json");
      return do_http("POST", url, hdrs, st.data_json.length() ? st.data_json : "{}",
                     j.ha_token);
    }
    case step_type::ha_webhook: {
      if (j.ha_base_url.isEmpty()) { set_msg("no HA base_url"); return false; }
      String url = j.ha_base_url + "/api/webhook/" + st.webhook_id;
      return do_http("POST", url, {}, "{}", "");
    }
    case step_type::hotkey:
    case step_type::type_text:
    case step_type::launch_app:
    case step_type::run_command:
    case step_type::media_key:
      return run_host_step(st);
    case step_type::delay:
      vTaskDelay(pdMS_TO_TICKS(st.delay_ms));
      return true;
    default:
      LOG_I("deck", "skipping unsupported step type");
      return true; // skipped steps are not failures
  }
}

static void worker(void *arg)
{
  job *j = (job *)arg;
  bool all_ok = true;
  set_msg("running...");
  for (const auto &st : j->steps) {
    if (!run_step(st, *j)) {
      all_ok = false;
      // best-effort: continue remaining steps (protocol section 4)
    }
  }
  if (all_ok) set_msg("done");
  s_ok = all_ok;
  s_active = false;
  s_busy = false;
  delete j;
  vTaskDelete(NULL);
}

void run(const deck_config::config &cfg, const deck_config::button &btn)
{
  if (s_busy) {
    LOG_W("deck", "executor busy; ignoring tap");
    return;
  }
  if (btn.steps.empty()) return;

  job *j = new job();
  j->steps = btn.steps;          // deep copy
  j->ha_base_url = cfg.ha_base_url;
  j->ha_token = cfg.ha_token;

  s_busy = true;
  s_active = true;
  s_ok = false;
  // 8 KB stack: HTTPClient over plain HTTP fits comfortably.
  xTaskCreate(worker, "deck_exec", 8192, j, 1, NULL);
}

bool busy() { return s_busy; }

status last_status()
{
  status s;
  s.active = s_active;
  s.ok = s_ok;
  strncpy(s.message, s_msg, sizeof(s.message) - 1);
  s.message[sizeof(s.message) - 1] = '\0';
  return s;
}

} // namespace deck_executor
