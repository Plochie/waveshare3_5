# WiFi Stream Deck — Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A config-driven Stream Deck app on the ESP32-S3 that renders a grid of
button tiles from an SD-card config and, on tap, runs a button's multi-step
workflow of **direct-network** actions (HTTP requests, Home Assistant service
calls, HA webhooks, delays) — with **zero desktop software**.

**Architecture:** Three new units. `core/deck_config` parses `/deck/config.json`
(ArduinoJson) into an in-memory model. `core/deck_executor` runs a button's
steps on a FreeRTOS worker task (so blocking HTTP + `delay` never stall LVGL),
using `HTTPClient`. `apps/deck` builds the LVGL grid from the model, handles
page navigation (folders) and taps, and is registered like every other app.
Host-only step types (hotkey/launch/etc.) are parsed but skipped this phase.

**Tech Stack:** C++ (Arduino-ESP32), LVGL v9.2.2, ArduinoJson v7, HTTPClient,
FreeRTOS tasks, SD via the existing `hal_sdcard`. Follows the repo's existing
app pattern (`apps/<name>/<name>.{h,cpp}` + `app_registry` + `eez_demo` nav).

## Global Constraints

- Verification is **build + on-device serial logs + manual hardware checks** —
  this repo has **no unit-test framework** (CLAUDE.md). Do not add one.
- Build command (PlatformIO not on PATH): `~/.platformio/penv/bin/pio run`.
  A task is "green" when this prints `[SUCCESS]`.
- **Git:** the repo has **0 commits and 0 tracked files**. Per repo policy,
  committing requires the user's go-ahead. Before running any task's commit
  step, confirm with the user and establish a baseline commit of the existing
  tree first (or skip git for this phase). The commit steps below assume that
  baseline exists.
- Use `styles::` colors (CLAUDE.md), root-relative includes (`-I src`), and the
  existing `screen_manager` push/pop + `app_registry` registration pattern.
- Step types this phase (direct only): `http_request`, `ha_service`,
  `ha_webhook`, `delay`. All other step types (`hotkey`, `type_text`,
  `launch_app`, `run_command`, `media_key`, `mqtt_publish`) are **parsed and
  skipped** (logged), per the protocol's forward-compat rule.
- Schema + protocol are fixed in
  `docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md`; match field
  names exactly.

---

## File structure

- Create `src/core/deck_config.h` / `.cpp` — model structs + `parse(String)` +
  `load()` (reads SD) + `find_page()` + `self_test()`.
- Create `src/core/deck_executor.h` / `.cpp` — `run(cfg, button)` worker task +
  `busy()` + `last_status()`; direct-step implementations.
- Create `src/apps/deck/deck.h` / `.cpp` — `deck_create()` grid UI + nav + taps.
- Modify `src/apps/app_registry.cpp` — register "Deck".
- Modify `src/apps/eez_demo/eez_demo.cpp` — add a "Deck" nav button.
- Author `/deck/config.json` on the SD card (sample content in Task 4) — not a
  repo file; placed on the card for hardware testing.

---

## Task 1: Deck config model + parser (`core/deck_config`)

**Files:**
- Create: `src/core/deck_config.h`
- Create: `src/core/deck_config.cpp`

**Interfaces:**
- Produces:
  - `enum class deck_config::step_type { unknown, http_request, ha_service, ha_webhook, mqtt_publish, hotkey, type_text, launch_app, run_command, media_key, delay }`
  - `struct deck_config::step` (fields below)
  - `struct deck_config::button { int pos; String label, icon, color, open_page; std::vector<step> steps; }`
  - `struct deck_config::page { String id, title; std::vector<button> buttons; }`
  - `struct deck_config::config { int version, cols, rows; String ha_base_url, ha_token; std::vector<page> pages; }`
  - `bool deck_config::parse(const String &json, config &out)`
  - `bool deck_config::load(config &out)` (reads `/deck/config.json`)
  - `const page *deck_config::find_page(const config &c, const String &id)`
  - `void deck_config::self_test()`

- [ ] **Step 1: Write `src/core/deck_config.h`**

```cpp
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
```

- [ ] **Step 2: Write `src/core/deck_config.cpp`**

```cpp
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
```

- [ ] **Step 3: Build**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]`. (deck_config.cpp compiles via LDF because it's reachable
once Task 3 includes it; to force-compile it now, it's fine that nothing
includes it yet — LDF deep+ skips unreferenced files, so a clean build still
passes. The real compile check for this file happens in Task 3. If you want an
immediate check, temporarily `#include "core/deck_config.h"` + call
`deck_config::self_test();` at the end of `setup()` in `src/main.cpp`, build,
then revert.)

- [ ] **Step 4: Verify the parser logic on hardware (optional but recommended)**

Temporarily add to `src/main.cpp` `setup()` after SD init:
`#include "core/deck_config.h"` and `deck_config::self_test();`
Flash: `~/.platformio/penv/bin/pio run -t upload -t monitor`
Expected serial lines:
`[deck] self_test: ver=1 grid=3x4 pages=1`
`[deck] self_test: home buttons=2 btn0 steps=2 btn1 open_page=work`
Then revert the `main.cpp` edit.

- [ ] **Step 5: Commit** (only after the git baseline is established — see Global Constraints)

```bash
git add src/core/deck_config.h src/core/deck_config.cpp
git commit -m "feat(deck): config model + JSON parser"
```

---

## Task 2: Direct-step executor (`core/deck_executor`)

**Files:**
- Create: `src/core/deck_executor.h`
- Create: `src/core/deck_executor.cpp`

**Interfaces:**
- Consumes: `deck_config::config`, `deck_config::button`, `deck_config::step`,
  `deck_config::step_type` (Task 1).
- Produces:
  - `void deck_executor::run(const deck_config::config &cfg, const deck_config::button &btn)`
  - `bool deck_executor::busy()`
  - `struct deck_executor::status { bool active; bool ok; char message[64]; }`
  - `deck_executor::status deck_executor::last_status()`

- [ ] **Step 1: Write `src/core/deck_executor.h`**

```cpp
#pragma once

#include "core/deck_config.h"

// Runs a button's step list on a background FreeRTOS task so blocking HTTP and
// delay steps never stall LVGL. Phase 1 executes only direct-network steps
// (http_request / ha_service / ha_webhook / delay); other step types are
// logged and skipped.
namespace deck_executor {

struct status {
  bool active;        // a sequence is currently running
  bool ok;            // result of the last finished sequence
  char message[64];   // short human-readable last-result message
};

// Starts running btn.steps. No-op (logged) if a run is already in progress.
// Copies everything it needs, so cfg/btn may change afterwards.
void run(const deck_config::config &cfg, const deck_config::button &btn);

bool busy();

// Snapshot of the current/last run status, safe to read from the LVGL thread.
status last_status();

} // namespace deck_executor
```

- [ ] **Step 2: Write `src/core/deck_executor.cpp`**

```cpp
#include "core/deck_executor.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "core/logging.h"

namespace deck_executor {

static constexpr uint16_t HTTP_TIMEOUT_MS = 3000;

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
    code = http.PUT(body);
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

static bool run_step(const deck_config::step &st, const job &j)
{
  using deck_config::step_type;
  switch (st.type) {
    case step_type::http_request: {
      std::vector<std::pair<String, String>> hdrs = st.headers;
      String body = st.body;
      if (st.method != "GET" && body.length() &&
          /* default content-type if none given */ true) {
        bool has_ct = false;
        for (auto &h : hdrs) if (h.first.equalsIgnoreCase("Content-Type")) has_ct = true;
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
    case step_type::delay:
      vTaskDelay(pdMS_TO_TICKS(st.delay_ms));
      return true;
    default:
      LOG_I("deck", "skipping host/unsupported step (phase 1)");
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
      // best-effort: continue remaining steps (protocol §4)
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
  // 8 KB stack: HTTPClient + TLS-less HTTP fits comfortably.
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
```

- [ ] **Step 3: Build**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]`. (Still not referenced by an app yet; LDF won't compile it
until Task 3 includes it. To force a compile check now, temporarily add
`#include "core/deck_executor.h"` to `src/main.cpp` and build, then revert. The
authoritative compile check is Task 3.)

- [ ] **Step 4: Commit** (after baseline — see Global Constraints)

```bash
git add src/core/deck_executor.h src/core/deck_executor.cpp
git commit -m "feat(deck): direct-network step executor on a worker task"
```

---

## Task 3: Deck grid app + registration (`apps/deck`)

**Files:**
- Create: `src/apps/deck/deck.h`
- Create: `src/apps/deck/deck.cpp`
- Modify: `src/apps/app_registry.cpp`
- Modify: `src/apps/eez_demo/eez_demo.cpp`

**Interfaces:**
- Consumes: `deck_config::{config,page,button,load,find_page}` (Task 1),
  `deck_executor::run` (Task 2), `styles::`, `screen_manager::`.
- Produces: `lv_obj_t *deck_create()`.

- [ ] **Step 1: Write `src/apps/deck/deck.h`**

```cpp
#pragma once

#include <lvgl.h>

// Creates the Stream Deck screen: a config-driven grid of button tiles loaded
// from /deck/config.json on SD. Tapping a tile runs its step workflow (direct-
// network actions) or navigates to another page (folder buttons).
lv_obj_t *deck_create();
```

- [ ] **Step 2: Write `src/apps/deck/deck.cpp`**

```cpp
#include "apps/deck/deck.h"

#include <vector>

#include "core/deck_config.h"
#include "core/deck_executor.h"
#include "core/screen_manager.h"
#include "ui/styles.h"

static deck_config::config s_cfg;
static bool s_loaded = false;
static std::vector<String> s_page_stack;   // navigation history of page ids
static lv_obj_t *s_root = nullptr;
static lv_obj_t *s_grid = nullptr;
static lv_obj_t *s_title = nullptr;
static lv_obj_t *s_status = nullptr;
static lv_obj_t *s_back = nullptr;
static lv_timer_t *s_status_timer = nullptr;

static const char *current_page_id()
{
  return s_page_stack.empty() ? "home" : s_page_stack.back().c_str();
}

static lv_color_t tile_color(const String &hex)
{
  if (hex.length() == 7 && hex[0] == '#') {
    long v = strtol(hex.c_str() + 1, nullptr, 16);
    return lv_color_hex((uint32_t)v);
  }
  return styles::bg_card();
}

// Forward decl.
static void rebuild_grid();

static void tile_click_cb(lv_event_t *e)
{
  // user_data encodes the button index within the current page.
  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  const deck_config::page *pg = deck_config::find_page(s_cfg, current_page_id());
  if (!pg || idx < 0 || idx >= (int)pg->buttons.size()) return;
  const deck_config::button &btn = pg->buttons[idx];
  if (btn.open_page.length()) {
    s_page_stack.push_back(btn.open_page);
    rebuild_grid();
  } else {
    deck_executor::run(s_cfg, btn);
  }
}

static void back_click_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_page_stack.size() > 1) {
    s_page_stack.pop_back();
    rebuild_grid();
  } else {
    screen_manager::pop(); // leave the app from the home page
  }
}

static void make_tile(lv_obj_t *parent, const deck_config::button &btn, int idx,
                      int w, int h)
{
  lv_obj_t *tile = lv_obj_create(parent);
  lv_obj_set_size(tile, w, h);
  lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(tile, 10, 0);
  lv_obj_set_style_bg_color(tile, tile_color(btn.color), 0);
  lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(tile, styles::border(), 0);
  lv_obj_set_style_border_width(tile, 1, 0);
  lv_obj_set_style_pad_all(tile, 4, 0);
  lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(tile, tile_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);

  lv_obj_t *label = lv_label_create(tile);
  lv_label_set_text(label, btn.label.c_str());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label, w - 12);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(label, styles::text_primary(), 0);
  lv_obj_center(label);
}

static void rebuild_grid()
{
  const deck_config::page *pg = deck_config::find_page(s_cfg, current_page_id());
  lv_label_set_text(s_title, pg ? pg->title.c_str() : "Deck");
  if (s_page_stack.size() > 1) {
    lv_label_set_text(s_back, LV_SYMBOL_LEFT " Back");
  } else {
    lv_label_set_text(s_back, LV_SYMBOL_LEFT " Exit");
  }

  lv_obj_clean(s_grid);
  if (!pg) return;

  const int cols = s_cfg.cols > 0 ? s_cfg.cols : 3;
  const int rows = s_cfg.rows > 0 ? s_cfg.rows : 4;
  const int gap = 8;
  const int gw = 320 - 2 * gap;
  const int gh = 480 - 40 /*header*/ - 24 /*status*/ - gap;
  const int tw = (gw - (cols - 1) * gap) / cols;
  const int th = (gh - (rows - 1) * gap) / rows;

  for (int i = 0; i < (int)pg->buttons.size(); i++) {
    const deck_config::button &btn = pg->buttons[i];
    if (btn.pos < 0 || btn.pos >= cols * rows) continue;
    int r = btn.pos / cols;
    int c = btn.pos % cols;
    int x = c * (tw + gap);
    int y = r * (th + gap);
    lv_obj_t *holder = lv_obj_create(s_grid); // positioned wrapper
    lv_obj_remove_flag(holder, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(holder, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(holder, 0, 0);
    lv_obj_set_style_pad_all(holder, 0, 0);
    lv_obj_set_size(holder, tw, th);
    lv_obj_set_pos(holder, x, y);
    make_tile(holder, btn, i, tw, th);
  }
}

static void status_timer_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  deck_executor::status st = deck_executor::last_status();
  if (st.active) {
    lv_label_set_text(s_status, st.message);
    lv_obj_set_style_text_color(s_status, styles::accent_amber(), 0);
  } else if (st.message[0]) {
    lv_label_set_text(s_status, st.message);
    lv_obj_set_style_text_color(s_status,
        st.ok ? styles::accent_green() : styles::accent_red(), 0);
  }
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_status_timer) {
    lv_timer_delete(s_status_timer);
    s_status_timer = nullptr;
  }
}

lv_obj_t *deck_create()
{
  s_root = lv_obj_create(NULL);
  lv_obj_add_style(s_root, &styles::style_screen, 0);
  lv_obj_remove_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(s_root, 0, 0);
  lv_obj_add_event_cb(s_root, screen_delete_cb, LV_EVENT_DELETE, NULL);

  // Header: back/exit + title.
  s_back = lv_label_create(s_root);
  lv_obj_set_pos(s_back, 8, 12);
  lv_obj_set_style_text_color(s_back, styles::accent_blue(), 0);
  lv_obj_add_flag(s_back, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_back, back_click_cb, LV_EVENT_CLICKED, NULL);

  s_title = lv_label_create(s_root);
  lv_obj_set_pos(s_title, 0, 12);
  lv_obj_set_width(s_title, 320);
  lv_obj_set_style_text_align(s_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(s_title, styles::text_secondary(), 0);

  // Grid container.
  s_grid = lv_obj_create(s_root);
  lv_obj_set_pos(s_grid, 8, 40);
  lv_obj_set_size(s_grid, 320 - 16, 480 - 40 - 24);
  lv_obj_remove_flag(s_grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(s_grid, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(s_grid, 0, 0);
  lv_obj_set_style_pad_all(s_grid, 0, 0);

  // Status line.
  s_status = lv_label_create(s_root);
  lv_obj_set_pos(s_status, 8, 480 - 20);
  lv_obj_set_width(s_status, 320 - 16);
  lv_obj_set_style_text_align(s_status, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(s_status, styles::text_muted(), 0);

  // Load config once (kept in a static for re-entry).
  if (!s_loaded) {
    if (deck_config::load(s_cfg)) {
      s_loaded = true;
    }
  }
  s_page_stack.clear();
  s_page_stack.push_back("home");

  if (!s_loaded || s_cfg.pages.empty()) {
    lv_label_set_text(s_title, "Deck");
    lv_label_set_text(s_status, "No /deck/config.json on SD");
  } else {
    rebuild_grid();
  }

  s_status_timer = lv_timer_create(status_timer_cb, 200, NULL);
  return s_root;
}
```

- [ ] **Step 3: Register the app in `src/apps/app_registry.cpp`**

Add the include alongside the other app includes:

```cpp
#include "apps/deck/deck.h"
```

Add the table entry (after the WLED entry):

```cpp
    {"Deck", "", app_category::network, deck_create},
```

- [ ] **Step 4: Add a home nav button in `src/apps/eez_demo/eez_demo.cpp`**

Add the include:

```cpp
#include "apps/deck/deck.h"
```

Add the callback (next to the other `open_*_cb` functions):

```cpp
static void open_deck_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(deck_create());
}
```

Add the nav button (after the WLED button, next free y = 470 — note the home
screen now scrolls; that's fine):

```cpp
  create_nav_button(objects.main, "Deck", 470, open_deck_cb);
```

- [ ] **Step 5: Build**

Run: `~/.platformio/penv/bin/pio run`
Expected: `[SUCCESS]`. This is the authoritative compile check for Tasks 1–3
(deck_config + deck_executor are now reachable from `apps/deck`).

- [ ] **Step 6: Commit** (after baseline — see Global Constraints)

```bash
git add src/apps/deck/deck.h src/apps/deck/deck.cpp src/apps/app_registry.cpp src/apps/eez_demo/eez_demo.cpp
git commit -m "feat(deck): config-driven grid app with page nav + tap-to-run"
```

---

## Task 4: Sample config + end-to-end hardware test

**Files:**
- Author on the SD card: `/deck/config.json` (not a repo file).

**Interfaces:** none (integration/verification task).

- [ ] **Step 1: Create the SD config**

Put this at `/deck/config.json` on the MicroSD card (edit `base_url`, the
webhook id, and the `http_request` URL to something reachable on your network).
Create an HA webhook automation with id `deck_test` for the webhook button, and
point the HTTP button at any endpoint you can watch (e.g. a local server log).

```json
{
  "version": 1,
  "grid": { "cols": 3, "rows": 4 },
  "ha": { "base_url": "http://homeassistant.local:8123", "token": "REPLACE_WITH_LONG_LIVED_TOKEN" },
  "pages": [
    {
      "id": "home",
      "title": "Home",
      "buttons": [
        { "pos": 0, "label": "HA Webhook", "color": "#3ECF8E",
          "steps": [ { "type": "ha_webhook", "id": "deck_test" } ] },
        { "pos": 1, "label": "Toggle Light", "color": "#60A5FA",
          "steps": [ { "type": "ha_service", "domain": "light", "service": "toggle",
                       "data": { "entity_id": "light.office" } } ] },
        { "pos": 2, "label": "Ping Server", "color": "#F59E0B",
          "steps": [ { "type": "http_request", "method": "POST",
                       "url": "http://192.168.1.50:8080/deck/ping",
                       "body": "{\"from\":\"deck\"}" } ] },
        { "pos": 3, "label": "Combo", "color": "#A78BFA",
          "steps": [ { "type": "ha_webhook", "id": "deck_test" },
                     { "type": "delay", "ms": 500 },
                     { "type": "http_request", "method": "GET",
                       "url": "http://192.168.1.50:8080/deck/after" } ] },
        { "pos": 5, "label": "Work", "color": "#2A2A2E", "open_page": "work" }
      ]
    },
    {
      "id": "work",
      "title": "Work",
      "buttons": [
        { "pos": 0, "label": "Build", "color": "#F87171",
          "steps": [ { "type": "http_request", "method": "POST",
                       "url": "http://192.168.1.50:8080/ci/build" } ] }
      ]
    }
  ]
}
```

- [ ] **Step 2: Flash + monitor**

Run: `~/.platformio/penv/bin/pio run -t upload -t monitor`

- [ ] **Step 3: Manual verification checklist**

On the device:
- Open the home screen → tap **Deck**. Expect a 3×4 grid with the 5 home tiles
  (slot 4 empty), colored per config, titled "Home".
- Tap **HA Webhook** → status line shows "running..." then "done" (green);
  serial logs `POST .../api/webhook/deck_test -> 200`; the HA automation fires.
- Tap **Toggle Light** → the light toggles; serial shows the services call + 200.
- Tap **Ping Server** → your server logs the POST; status "done".
- Tap **Combo** → webhook fires, ~0.5 s pause, then the GET; status "done".
- Tap **Work** → grid switches to the Work page, header shows "Back"; tap
  **Back** → returns home; from home, **Exit** leaves the app.
- Tap a button targeting an unreachable URL → status shows "HTTP …" / error in
  red; the UI stays responsive (proves the worker task isn't blocking LVGL).

- [ ] **Step 4: Commit the plan progress** (no repo files changed here; nothing
  to commit unless you tracked the sample config elsewhere).

---

## Self-review notes

- **Spec coverage (Phase 1 rows of the design doc):** grid UI from SD config
  (Task 3) ✓; page nav/folders (Task 3) ✓; executor off the UI thread (Task 2)
  ✓; direct steps http_request/ha_service/ha_webhook/delay (Task 2) ✓; host
  steps parsed-and-skipped (Tasks 1–2) ✓; status feedback via lv_timer (Task 3)
  ✓; config hand-placed on SD (Task 4) ✓.
- **Deferred (correct for Phase 1):** WebSocket client, icon `.bin` rendering,
  mDNS connect to agent, host-action execution — all later phases. Icons: tiles
  render label+color only this phase (the `icon` field is parsed but unused
  until the icon pipeline exists in Phase 2).
- **Type consistency:** `deck_config::{step,button,page,config}` field names and
  `deck_executor::{run,busy,last_status,status}` signatures match across Tasks
  1–3.
- **No placeholders:** every code step contains complete, compilable code.
