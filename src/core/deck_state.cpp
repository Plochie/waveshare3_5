#include "core/deck_state.h"

#include <map>

#include "core/deck_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace deck_state {

static std::map<String, String> s_map;
static SemaphoreHandle_t s_mtx = nullptr;
static volatile bool s_changed = false;

void init()
{
  if (!s_mtx) s_mtx = xSemaphoreCreateMutex();
}

void set_local(const String &key, const String &value)
{
  if (!s_mtx || key.isEmpty()) return;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  s_map[key] = value;
  xSemaphoreGive(s_mtx);
  s_changed = true;
}

String get(const String &key)
{
  String out;
  if (!s_mtx) return out;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  auto it = s_map.find(key);
  if (it != s_map.end()) out = it->second;
  xSemaphoreGive(s_mtx);
  return out;
}

bool has(const String &key)
{
  if (!s_mtx) return false;
  xSemaphoreTake(s_mtx, portMAX_DELAY);
  bool h = s_map.find(key) != s_map.end();
  xSemaphoreGive(s_mtx);
  return h;
}

void set_and_send(const String &key, const String &value)
{
  set_local(key, value);
  deck_client::send_set(key, value);
}

bool consume_changed()
{
  if (!s_changed) return false;
  s_changed = false;
  return true;
}

} // namespace deck_state
