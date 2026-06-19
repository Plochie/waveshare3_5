#include "core/wifi_store.h"

#include "hal_sdcard.h"

namespace wifi_store {

static constexpr const char *PATH = "/wifi_known.txt";

size_t load(credential out[], size_t max_count)
{
  if (!sdcard::is_mounted()) {
    return 0;
  }

  String content = sdcard::read_string(PATH);
  size_t count = 0;
  int pos = 0;
  while (count < max_count && pos < (int)content.length()) {
    int nl1 = content.indexOf('\n', pos);
    if (nl1 < 0) {
      break; // truncated/missing password line - stop
    }
    String ssid = content.substring(pos, nl1);
    pos = nl1 + 1;

    int nl2 = content.indexOf('\n', pos);
    String password;
    if (nl2 < 0) {
      password = content.substring(pos);
      pos = content.length();
    } else {
      password = content.substring(pos, nl2);
      pos = nl2 + 1;
    }

    if (ssid.length() == 0) {
      continue;
    }
    out[count].ssid = ssid;
    out[count].password = password;
    count++;
  }
  return count;
}

void save(const char *ssid, const char *password)
{
  if (!sdcard::is_mounted() || !ssid || !ssid[0]) {
    return;
  }

  credential existing[MAX_NETWORKS];
  size_t existing_count = load(existing, MAX_NETWORKS);

  // New/updated entry goes first; copy over the rest, dropping any prior
  // entry for the same SSID and capping the total at MAX_NETWORKS.
  credential merged[MAX_NETWORKS];
  size_t out_count = 0;
  merged[out_count].ssid = ssid;
  merged[out_count].password = password ? password : "";
  out_count++;
  for (size_t i = 0; i < existing_count && out_count < MAX_NETWORKS; i++) {
    if (existing[i].ssid == ssid) {
      continue;
    }
    merged[out_count++] = existing[i];
  }

  String content;
  for (size_t i = 0; i < out_count; i++) {
    content += merged[i].ssid;
    content += '\n';
    content += merged[i].password;
    content += '\n';
  }
  sdcard::write(PATH, content.c_str());
}

} // namespace wifi_store
