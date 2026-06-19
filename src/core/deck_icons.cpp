#include "core/deck_icons.h"

#include <esp_heap_caps.h>

#include "core/logging.h"
#include "hal_sdcard.h"

namespace deck_icons {

static constexpr const char *kDir = "/deck/icons";

static bool s_recv_active = false;
static String s_recv_path;
static int s_recv_w = 0;
static int s_recv_h = 0;

static String icon_path(const String &name)
{
  return String(kDir) + "/" + name;
}

static void collect_names_cb(const char *name, bool is_dir, size_t size, void *user)
{
  (void)size;
  if (is_dir) return;
  String n(name);
  if (n.endsWith(".json")) return; // dims sidecar, not an icon itself
  static_cast<std::vector<String> *>(user)->push_back(n);
}

std::vector<String> all_names()
{
  std::vector<String> names;
  sdcard::list(kDir, collect_names_cb, &names);
  return names;
}

bool get_dims(const String &name, int &w, int &h)
{
  String json = sdcard::read_string((icon_path(name) + ".json").c_str());
  if (json.isEmpty()) return false;
  int wi = json.indexOf("\"w\":");
  int hi = json.indexOf("\"h\":");
  if (wi < 0 || hi < 0) return false;
  w = json.substring(wi + 4).toInt();
  h = json.substring(hi + 4).toInt();
  return w > 0 && h > 0;
}

bool begin_receive(const String &name, size_t total_bytes, int w, int h)
{
  (void)total_bytes;
  sdcard::mkdir("/deck");
  sdcard::mkdir(kDir);
  s_recv_path = icon_path(name);
  if (!sdcard::write(s_recv_path.c_str(), (const uint8_t *)nullptr, 0)) {
    LOG_E("deck_icons", "failed to open %s for writing", s_recv_path.c_str());
    s_recv_active = false;
    return false;
  }
  s_recv_w = w;
  s_recv_h = h;
  s_recv_active = true;
  return true;
}

bool write_chunk(const uint8_t *data, size_t len)
{
  if (!s_recv_active) return false;
  return sdcard::append(s_recv_path.c_str(), data, len);
}

bool end_receive()
{
  if (!s_recv_active) return false;
  s_recv_active = false;
  String json = String("{\"w\":") + s_recv_w + ",\"h\":" + s_recv_h + "}";
  return sdcard::write((s_recv_path + ".json").c_str(), json.c_str());
}

uint8_t *load_pixels(const String &name, size_t &len)
{
  len = 0;
  fs::File f = sdcard::fs().open(icon_path(name).c_str(), FILE_READ);
  if (!f) return nullptr;
  size_t sz = f.size();
  if (sz == 0) {
    f.close();
    return nullptr;
  }
  uint8_t *buf = (uint8_t *)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf) {
    LOG_E("deck_icons", "PSRAM alloc failed for %s (%u bytes)", name.c_str(),
          (unsigned)sz);
    f.close();
    return nullptr;
  }
  size_t n = f.read(buf, sz);
  f.close();
  if (n != sz) {
    free(buf);
    return nullptr;
  }
  len = sz;
  return buf;
}

} // namespace deck_icons
