#include "core/settings_store.h"

#include <Arduino.h>

#include "hal_sdcard.h"

namespace settings_store {

static constexpr const char *PATH = "/settings.txt";
static constexpr settings_t DEFAULTS = {100, 30};

settings_t load()
{
  settings_t s = DEFAULTS;
  if (!sdcard::is_mounted()) {
    return s;
  }

  String content = sdcard::read_string(PATH);
  int pos = 0;
  while (pos < (int)content.length()) {
    int nl = content.indexOf('\n', pos);
    String line = (nl < 0) ? content.substring(pos) : content.substring(pos, nl);
    pos = (nl < 0) ? content.length() : nl + 1;

    int eq = line.indexOf('=');
    if (eq < 0) {
      continue;
    }
    String key = line.substring(0, eq);
    String value = line.substring(eq + 1);

    if (key == "brightness") {
      int v = value.toInt();
      if (v >= 10 && v <= 100) {
        s.brightness = (uint8_t)v;
      }
    } else if (key == "dim_timeout") {
      int v = value.toInt();
      if (v >= 0 && v <= 65535) {
        s.dim_timeout = (uint16_t)v;
      }
    }
  }
  return s;
}

void save(const settings_t &s)
{
  if (!sdcard::is_mounted()) {
    return;
  }

  String content;
  content += "brightness=";
  content += s.brightness;
  content += '\n';
  content += "dim_timeout=";
  content += s.dim_timeout;
  content += '\n';
  sdcard::write(PATH, content.c_str());
}

} // namespace settings_store
