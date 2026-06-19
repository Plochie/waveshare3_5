#include "core/wled_store.h"

#include "hal_sdcard.h"

namespace wled_store {

static constexpr const char *PATH = "/wled.txt";

// Each device is three consecutive lines: host, name, leds.
size_t load(device out[], size_t max_count)
{
  if (!sdcard::is_mounted()) {
    return 0;
  }

  String content = sdcard::read_string(PATH);
  size_t count = 0;
  int pos = 0;
  while (count < max_count && pos < (int)content.length()) {
    int nl1 = content.indexOf('\n', pos);
    if (nl1 < 0) break;
    String host = content.substring(pos, nl1);
    pos = nl1 + 1;

    int nl2 = content.indexOf('\n', pos);
    if (nl2 < 0) break;
    String name = content.substring(pos, nl2);
    pos = nl2 + 1;

    int nl3 = content.indexOf('\n', pos);
    String leds_str;
    if (nl3 < 0) {
      leds_str = content.substring(pos);
      pos = content.length();
    } else {
      leds_str = content.substring(pos, nl3);
      pos = nl3 + 1;
    }

    if (host.length() == 0) continue;
    out[count].host = host;
    out[count].name = name;
    out[count].leds = (uint16_t)leds_str.toInt();
    count++;
  }
  return count;
}

static void write_all(const device merged[], size_t count)
{
  String content;
  for (size_t i = 0; i < count; i++) {
    content += merged[i].host;
    content += '\n';
    content += merged[i].name;
    content += '\n';
    content += merged[i].leds;
    content += '\n';
  }
  sdcard::write(PATH, content.c_str());
}

void save(const char *host, const char *name, uint16_t leds)
{
  if (!sdcard::is_mounted() || !host || !host[0]) {
    return;
  }

  device existing[MAX_DEVICES];
  size_t existing_count = load(existing, MAX_DEVICES);

  // New/updated entry goes first; copy the rest, dropping any prior entry for
  // the same host and capping at MAX_DEVICES.
  device merged[MAX_DEVICES];
  size_t out_count = 0;
  merged[out_count].host = host;
  merged[out_count].name = (name && name[0]) ? name : host;
  merged[out_count].leds = leds;
  out_count++;
  for (size_t i = 0; i < existing_count && out_count < MAX_DEVICES; i++) {
    if (existing[i].host == host) continue;
    merged[out_count++] = existing[i];
  }

  write_all(merged, out_count);
}

void remove(const char *host)
{
  if (!sdcard::is_mounted() || !host || !host[0]) {
    return;
  }

  device existing[MAX_DEVICES];
  size_t existing_count = load(existing, MAX_DEVICES);

  device merged[MAX_DEVICES];
  size_t out_count = 0;
  for (size_t i = 0; i < existing_count; i++) {
    if (existing[i].host == host) continue;
    merged[out_count++] = existing[i];
  }
  write_all(merged, out_count);
}

} // namespace wled_store
