#include "core/deck_client.h"

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <esp_system.h>

#include "core/deck_config.h"
#include "core/deck_icons.h"
#include "core/deck_state.h"
#include "core/logging.h"
#include "hal_sdcard.h"

namespace deck_client {

static constexpr const char *FW_VERSION = "1.0.0";
static constexpr uint32_t REDISCOVER_INTERVAL_MS = 4000;

static WebSocketsClient s_ws;
static bool s_started = false;
static bool s_mdns_started = false;
static volatile bool s_authed = false;
static volatile bool s_config_pending = false;
static volatile bool s_icons_pending = false;
static String s_device_id;

// Host-step (exec/result) round trip — see exec_host_step()/handle_result().
static SemaphoreHandle_t s_exec_sem = nullptr;
static volatile uint32_t s_pending_exec_id = 0;
static volatile bool s_pending_ok = false;
static char s_pending_err[64] = {0};

// Forward declaration: defined further down (pairing token persistence), used
// by handle_paired() below.
static void save_pairing(const String &device_id, const String &token);

// Pairing handshake state (see pairing_state()/handle_pair_*).
static volatile pairing_state_t s_pair_state = PAIR_IDLE;
static char s_pair_code[7] = {0};

static void gen_pair_code()
{
  uint32_t r = esp_random() % 1000000u;
  snprintf(s_pair_code, sizeof(s_pair_code), "%06u", (unsigned)r);
}

static void send_pair_request()
{
  JsonDocument doc;
  doc["t"] = "pair_request";
  doc["device_id"] = s_device_id;
  doc["code"] = s_pair_code;
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}

static void handle_pair_required()
{
  gen_pair_code();
  s_pair_state = PAIR_PENDING;
  LOG_I("deck_client", "pairing required, code %s", s_pair_code);
  send_pair_request();
}

static void handle_paired(JsonDocument &doc)
{
  String token = (const char *)(doc["token"] | "");
  if (token.isEmpty()) return;
  save_pairing(s_device_id, token);
  s_authed = true;
  s_pair_state = PAIR_SUCCESS;
  LOG_I("deck_client", "paired");
}

static void handle_pair_rejected()
{
  s_pair_state = PAIR_REJECTED;
  LOG_W("deck_client", "pairing rejected by desktop");
}

static void set_pending_err(const char *msg)
{
  strncpy(s_pending_err, msg, sizeof(s_pending_err) - 1);
  s_pending_err[sizeof(s_pending_err) - 1] = '\0';
}

static String make_device_id()
{
  String mac = WiFi.macAddress(); // "AA:BB:CC:DD:EE:FF"
  mac.replace(":", "");
  mac.toLowerCase();
  String tail = mac.length() >= 6 ? mac.substring(mac.length() - 6) : mac;
  return "esp32-" + tail;
}

static constexpr const char *PAIRING_PATH = "/deck/pairing.json";

// The device's own pairing token, persisted separately from the synced config
// (which is overwritten on every push). Empty string => unpaired.
static String load_agent_token()
{
  if (!sdcard::is_mounted() || !sdcard::exists(PAIRING_PATH)) return "";
  String json = sdcard::read_string(PAIRING_PATH);
  JsonDocument doc;
  if (deserializeJson(doc, json)) return "";
  return (const char *)(doc["token"] | "");
}

static void save_pairing(const String &device_id, const String &token)
{
  sdcard::mkdir("/deck"); // no-op if it already exists
  JsonDocument doc;
  doc["device_id"] = device_id;
  doc["token"] = token;
  String out;
  serializeJson(doc, out);
  if (sdcard::write(PAIRING_PATH, out.c_str())) {
    LOG_I("deck_client", "pairing token saved");
  } else {
    LOG_E("deck_client", "failed to write %s", PAIRING_PATH);
  }
}

static bool discover(String &host, uint16_t &port, String &ws_path)
{
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!s_mdns_started) {
    MDNS.begin("waveshare-deck");
    s_mdns_started = true;
  }
  int n = MDNS.queryService("deckhost", "tcp");
  if (n <= 0) return false;
  IPAddress ip = MDNS.address(0);
  if (ip == IPAddress(0, 0, 0, 0)) return false;
  host = ip.toString();
  port = MDNS.port(0);
  ws_path = MDNS.hasTxt(0, "ws_path") ? MDNS.txt(0, "ws_path") : "/ws";
  return true;
}

static void send_hello()
{
  JsonDocument doc;
  doc["t"] = "hello";
  doc["device_id"] = s_device_id;
  doc["fw"] = FW_VERSION;
  doc["token"] = load_agent_token();
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}

// Tells the agent which icons are already on SD (protocol §3.3) so it only
// pushes the ones we're missing. Sent right after auth_ok.
static void send_have_icons()
{
  JsonDocument doc;
  doc["t"] = "have_icons";
  JsonArray names = doc["names"].to<JsonArray>();
  for (const String &n : deck_icons::all_names()) {
    names.add(n);
  }
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}

static void handle_icon_begin(JsonDocument &doc)
{
  String name = doc["name"] | "";
  size_t bytes = doc["bytes"] | 0;
  int w = doc["w"] | 0;
  int h = doc["h"] | 0;
  if (name.isEmpty()) return;
  if (!deck_icons::begin_receive(name, bytes, w, h)) {
    LOG_E("deck_client", "failed to start icon receive for %s", name.c_str());
  }
}

static void handle_icon_end(JsonDocument &doc)
{
  String name = doc["name"] | "";
  if (name.isEmpty()) return;
  if (deck_icons::end_receive()) {
    s_icons_pending = true;
    LOG_I("deck_client", "icon %s received", name.c_str());
    JsonDocument ack;
    ack["t"] = "icon_ack";
    ack["name"] = name;
    String out;
    serializeJson(ack, out);
    s_ws.sendTXT(out);
  } else {
    LOG_E("deck_client", "icon %s: end_receive failed", name.c_str());
  }
}

// Matches a `result` (protocol §3.4) against the exec currently awaited by
// exec_host_step(), if any, and wakes it up. Runs on this task (the one
// driving s_ws.loop()), exec_host_step() runs on deck_executor's task.
static void handle_result(JsonDocument &doc)
{
  uint32_t id = doc["id"] | 0;
  if (id == 0 || id != s_pending_exec_id) return; // stale or unexpected
  s_pending_ok = doc["ok"] | false;
  set_pending_err((const char *)(doc["error"] | ""));
  if (s_exec_sem) xSemaphoreGive(s_exec_sem);
}

static void handle_config(JsonDocument &doc)
{
  String json;
  serializeJson(doc["config"], json);
  sdcard::mkdir("/deck"); // no-op if it already exists
  if (sdcard::write("/deck/config.json", json.c_str())) {
    s_config_pending = true;
    LOG_I("deck_client", "config pushed (%u bytes), saved to SD",
          (unsigned)json.length());
  } else {
    LOG_E("deck_client", "failed to write /deck/config.json");
  }

  JsonDocument ack;
  ack["t"] = "config_ack";
  ack["rev"] = doc["rev"] | 0;
  String out;
  serializeJson(ack, out);
  s_ws.sendTXT(out);
}

static void handle_message(const uint8_t *payload, size_t len)
{
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, len);
  if (err) {
    LOG_W("deck_client", "bad json from agent: %s", err.c_str());
    return;
  }
  const char *t = doc["t"] | "";
  if (!strcmp(t, "auth_ok")) {
    s_authed = true;
    LOG_I("deck_client", "authenticated with agent");
    send_have_icons();
  } else if (!strcmp(t, "auth_fail")) {
    s_authed = false;
    LOG_W("deck_client", "auth_fail: %s", (const char *)(doc["reason"] | ""));
  } else if (!strcmp(t, "config")) {
    handle_config(doc);
  } else if (!strcmp(t, "icon_begin")) {
    handle_icon_begin(doc);
  } else if (!strcmp(t, "icon_end")) {
    handle_icon_end(doc);
  } else if (!strcmp(t, "result")) {
    handle_result(doc);
  } else if (!strcmp(t, "pair_required")) {
    handle_pair_required();
  } else if (!strcmp(t, "paired")) {
    handle_paired(doc);
  } else if (!strcmp(t, "pair_rejected")) {
    handle_pair_rejected();
  } else if (!strcmp(t, "state_snapshot")) {
    for (JsonPairConst kv : doc["values"].as<JsonObjectConst>()) {
      deck_state::set_local(kv.key().c_str(), (const char *)(kv.value() | ""));
    }
  } else if (!strcmp(t, "state")) {
    deck_state::set_local((const char *)(doc["key"] | ""),
                          (const char *)(doc["value"] | ""));
  }
  // Unknown `t` values are ignored for forward-compat (per protocol §5).
}

static void on_event(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type) {
    case WStype_CONNECTED:
      s_authed = false;
      s_pair_state = PAIR_IDLE;
      LOG_I("deck_client", "connected to agent, sending hello");
      send_hello();
      break;
    case WStype_DISCONNECTED:
      s_authed = false;
      LOG_W("deck_client", "disconnected from agent");
      // Unblock any exec_host_step() wait — it'll never see its `result` now.
      if (s_pending_exec_id != 0 && s_exec_sem) {
        s_pending_ok = false;
        set_pending_err("disconnected");
        xSemaphoreGive(s_exec_sem);
      }
      if (s_pair_state == PAIR_PENDING) s_pair_state = PAIR_IDLE; // pop the pairing screen
      break;
    case WStype_TEXT:
      handle_message(payload, length);
      break;
    case WStype_BIN:
      deck_icons::write_chunk(payload, length);
      break;
    default:
      break;
  }
}

static void client_task(void *arg)
{
  (void)arg;
  s_ws.onEvent(on_event);

  String host, ws_path;
  uint16_t port = 0;
  uint32_t last_discover_ms = 0;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      s_authed = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    if (!s_ws.isConnected()) {
      uint32_t now = millis();
      if (now - last_discover_ms > REDISCOVER_INTERVAL_MS) {
        last_discover_ms = now;
        if (discover(host, port, ws_path)) {
          LOG_I("deck_client", "found agent at %s:%u%s", host.c_str(), port,
                ws_path.c_str());
          s_ws.begin(host.c_str(), port, ws_path.c_str());
        }
      }
    }
    s_ws.loop();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void start()
{
  if (s_started) return;
  s_started = true;
  s_device_id = make_device_id();
  s_exec_sem = xSemaphoreCreateBinary();
  xTaskCreate(client_task, "deck_client", 8192, NULL, 1, NULL);
}

bool connected()
{
  return s_authed;
}

bool consume_config_pushed()
{
  if (!s_config_pending) return false;
  s_config_pending = false;
  return true;
}

bool consume_icons_pushed()
{
  if (!s_icons_pending) return false;
  s_icons_pending = false;
  return true;
}

static uint32_t s_next_exec_id = 0;

bool exec_host_step(const String &step_json, uint32_t timeout_ms, String &err)
{
  if (!s_authed || !s_ws.isConnected()) {
    err = "not connected";
    return false;
  }

  uint32_t id = ++s_next_exec_id;
  if (id == 0) id = ++s_next_exec_id; // skip the sentinel "no exec" value
  s_pending_exec_id = id;
  xSemaphoreTake(s_exec_sem, 0); // drop any stale signal from a prior exec

  String out = "{\"t\":\"exec\",\"id\":" + String(id) + ",\"step\":" + step_json + "}";
  s_ws.sendTXT(out);

  bool signaled = xSemaphoreTake(s_exec_sem, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
  s_pending_exec_id = 0;
  if (!signaled) {
    err = "timeout";
    return false;
  }
  if (!s_pending_ok) {
    err = s_pending_err[0] ? s_pending_err : "step failed";
  }
  return s_pending_ok;
}

pairing_status pairing_state()
{
  pairing_status s;
  s.state = s_pair_state;
  strncpy(s.code, s_pair_code, sizeof(s.code) - 1);
  s.code[sizeof(s.code) - 1] = '\0';
  return s;
}

void clear_pairing()
{
  s_pair_state = PAIR_IDLE;
}

void send_set(const String &key, const String &value)
{
  if (!s_authed || !s_ws.isConnected() || key.isEmpty()) return;
  JsonDocument doc;
  doc["t"] = "set";
  doc["key"] = key;
  doc["value"] = value;
  String out;
  serializeJson(doc, out);
  s_ws.sendTXT(out);
}

} // namespace deck_client
