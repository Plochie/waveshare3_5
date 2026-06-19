#include "hal_sdcard.h"

#include "board_config.h"

namespace sdcard {

static bool s_mounted = false;

bool init()
{
  if (s_mounted) {
    return true;
  }

  // SD_MMC defaults to ESP32-S3 flash-conflicting pins, so remap first.
  if (!SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN)) {
    log_e("SD_MMC.setPins() failed");
    return false;
  }

  // mode1bit = true -> 1-bit SDIO (only D0 wired on this board).
  if (!SD_MMC.begin("/sdcard", /*mode1bit=*/true)) {
    log_e("SD_MMC.begin() failed (no card / bad wiring?)");
    return false;
  }

  if (SD_MMC.cardType() == CARD_NONE) {
    log_e("No SD card detected");
    SD_MMC.end();
    return false;
  }

  s_mounted = true;
  return true;
}

void deinit()
{
  if (s_mounted) {
    SD_MMC.end();
    s_mounted = false;
  }
}

bool is_mounted()
{
  return s_mounted;
}

uint64_t size_bytes()
{
  return s_mounted ? SD_MMC.cardSize() : 0;
}

uint64_t total_bytes()
{
  return s_mounted ? SD_MMC.totalBytes() : 0;
}

uint64_t used_bytes()
{
  return s_mounted ? SD_MMC.usedBytes() : 0;
}

const char *type_str()
{
  if (!s_mounted) {
    return "NONE";
  }
  switch (SD_MMC.cardType()) {
    case CARD_MMC:  return "MMC";
    case CARD_SD:   return "SDSC";
    case CARD_SDHC: return "SDHC";
    case CARD_NONE: return "NONE";
    default:        return "UNKNOWN";
  }
}

fs::FS &fs()
{
  return SD_MMC;
}

bool exists(const char *path)
{
  return s_mounted && SD_MMC.exists(path);
}

static bool write_mode(const char *path, const uint8_t *data, size_t len,
                       const char *mode)
{
  if (!s_mounted) {
    return false;
  }
  File f = SD_MMC.open(path, mode);
  if (!f) {
    log_e("open('%s', '%s') failed", path, mode);
    return false;
  }
  size_t written = (len > 0) ? f.write(data, len) : 0;
  f.close();
  return written == len;
}

bool write(const char *path, const uint8_t *data, size_t len)
{
  return write_mode(path, data, len, FILE_WRITE);
}

bool write(const char *path, const char *text)
{
  return write_mode(path, reinterpret_cast<const uint8_t *>(text),
                    text ? strlen(text) : 0, FILE_WRITE);
}

bool append(const char *path, const uint8_t *data, size_t len)
{
  return write_mode(path, data, len, FILE_APPEND);
}

bool append(const char *path, const char *text)
{
  return write_mode(path, reinterpret_cast<const uint8_t *>(text),
                    text ? strlen(text) : 0, FILE_APPEND);
}

int read(const char *path, uint8_t *buf, size_t maxlen)
{
  if (!s_mounted) {
    return -1;
  }
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return -1;
  }
  int n = static_cast<int>(f.read(buf, maxlen));
  f.close();
  return n;
}

String read_string(const char *path)
{
  if (!s_mounted) {
    return String();
  }
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return String();
  }
  String s = f.readString();
  f.close();
  return s;
}

bool remove(const char *path)
{
  return s_mounted && SD_MMC.remove(path);
}

bool rename(const char *from, const char *to)
{
  return s_mounted && SD_MMC.rename(from, to);
}

bool mkdir(const char *path)
{
  return s_mounted && SD_MMC.mkdir(path);
}

bool rmdir(const char *path)
{
  return s_mounted && SD_MMC.rmdir(path);
}

bool list(const char *dir,
          void (*cb)(const char *name, bool is_dir, size_t size, void *user),
          void *user)
{
  if (!s_mounted) {
    return false;
  }
  File root = SD_MMC.open(dir);
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    return false;
  }
  for (File entry = root.openNextFile(); entry; entry = root.openNextFile()) {
    if (cb) {
      cb(entry.name(), entry.isDirectory(), entry.size(), user);
    }
    entry.close();
  }
  root.close();
  return true;
}

bool self_test()
{
  if (!init()) {
    Serial.println("[sdcard] mount failed");
    return false;
  }

  Serial.printf("[sdcard] type=%s size=%lluMB used=%lluMB/%lluMB\n", type_str(),
                size_bytes() / (1024 * 1024), used_bytes() / (1024 * 1024),
                total_bytes() / (1024 * 1024));

  const char *path = "/.hal_sdcard_selftest";
  const char *msg = "sdcard ok";
  if (!write(path, msg)) {
    Serial.println("[sdcard] write failed");
    return false;
  }

  String back = read_string(path);
  remove(path);

  bool ok = (back == msg);
  Serial.printf("[sdcard] write/read round-trip: %s (read '%s')\n",
                ok ? "OK" : "MISMATCH", back.c_str());
  return ok;
}

} // namespace sdcard
