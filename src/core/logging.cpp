#include "core/logging.h"

#include <esp_log.h>
#include <stdio.h>
#include <string.h>

#include "hal_sdcard.h"

namespace logging {

// --- Tunables (per the agreed design: 256 KB x 30 files, per-boot rotation) ---
static constexpr const char *LOG_DIR = "/logs";
static constexpr size_t MAX_FILE_BYTES = 256 * 1024;
static constexpr int MAX_FILES = 30;
static constexpr size_t RING_LINES = 250;
static constexpr size_t LINE_LEN = 120;

// --- Ring buffer ---
static char s_lines[RING_LINES][LINE_LEN];
static uint8_t s_levels[RING_LINES];
static size_t s_head = 0;     // next slot to write
static size_t s_held = 0;     // lines currently held (<= RING_LINES)
static uint32_t s_revision = 0;

// --- SD file state ---
static File s_file;
static bool s_file_open = false;
static size_t s_file_bytes = 0;
static int s_file_index = 0;
static char s_file_path[48] = "";
static uint32_t s_last_flush_ms = 0;

static bool s_inited = false;
static volatile bool s_in_log = false; // re-entrancy guard (esp_log can recurse)

// ---------------------------------------------------------------------------

static int parse_index(const char *name)
{
  // Matches "log_0007.txt" -> 7; returns -1 otherwise.
  int idx = -1;
  if (sscanf(name, "log_%d.txt", &idx) == 1) {
    return idx;
  }
  return -1;
}

static void prune_old_files()
{
  // Keep only the newest MAX_FILES log_*.txt files (highest indices).
  fs::FS &fs = sdcard::fs();
  File dir = fs.open(LOG_DIR);
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return;
  }

  int indices[128];
  int n = 0;
  for (File e = dir.openNextFile(); e; e = dir.openNextFile()) {
    int idx = parse_index(e.name());
    if (idx >= 0 && n < (int)(sizeof(indices) / sizeof(indices[0]))) {
      indices[n++] = idx;
    }
    e.close();
  }
  dir.close();

  while (n > MAX_FILES) {
    // find + delete the smallest index
    int min_i = 0;
    for (int i = 1; i < n; i++) {
      if (indices[i] < indices[min_i]) min_i = i;
    }
    char path[48];
    snprintf(path, sizeof(path), "%s/log_%04d.txt", LOG_DIR, indices[min_i]);
    fs.remove(path);
    indices[min_i] = indices[--n]; // remove from array
  }
}

static int next_file_index()
{
  fs::FS &fs = sdcard::fs();
  File dir = fs.open(LOG_DIR);
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return 1;
  }
  int max_idx = 0;
  for (File e = dir.openNextFile(); e; e = dir.openNextFile()) {
    int idx = parse_index(e.name());
    if (idx > max_idx) max_idx = idx;
    e.close();
  }
  dir.close();
  return max_idx + 1;
}

static void open_new_file()
{
  if (!sdcard::is_mounted()) {
    return;
  }
  if (s_file_open) {
    s_file.close();
    s_file_open = false;
  }
  snprintf(s_file_path, sizeof(s_file_path), "%s/log_%04d.txt", LOG_DIR,
           s_file_index);
  s_file = sdcard::fs().open(s_file_path, FILE_WRITE);
  if (s_file) {
    s_file_open = true;
    s_file_bytes = 0;
    prune_old_files();
  } else {
    s_file_path[0] = '\0';
  }
}

static void sd_write_line(const char *line)
{
  if (!s_file_open) {
    return;
  }
  size_t len = strlen(line);
  s_file.write(reinterpret_cast<const uint8_t *>(line), len);
  s_file.write('\n');
  s_file_bytes += len + 1;

  if (s_file_bytes >= MAX_FILE_BYTES) {
    s_file_index++;
    open_new_file(); // rotate
    return;
  }

  uint32_t now = millis();
  if (now - s_last_flush_ms >= 1500) {
    s_file.flush();
    s_last_flush_ms = now;
  }
}

// Stores one fully-formatted line. echo_serial=false when the caller already
// printed it (e.g. the esp_log hook forwards the raw line itself).
static void store(level lvl, const char *line, bool echo_serial)
{
  if (s_in_log) {
    // Avoid recursion if SD I/O below itself emits an esp_log line.
    if (echo_serial) Serial.println(line);
    return;
  }
  s_in_log = true;

  if (echo_serial) {
    Serial.println(line);
  }

  // ring buffer
  strlcpy(s_lines[s_head], line, LINE_LEN);
  s_levels[s_head] = (uint8_t)lvl;
  s_head = (s_head + 1) % RING_LINES;
  if (s_held < RING_LINES) s_held++;
  s_revision++;

  // SD
  sd_write_line(line);

  s_in_log = false;
}

static void strip_ansi_nl(const char *in, char *out, size_t outlen)
{
  size_t o = 0;
  for (size_t i = 0; in[i] && o + 1 < outlen; i++) {
    if (in[i] == '\033') { // skip ANSI escape: ESC ... 'm'
      while (in[i] && in[i] != 'm') i++;
      continue;
    }
    if (in[i] == '\n' || in[i] == '\r') continue;
    out[o++] = in[i];
  }
  out[o] = '\0';
}

static level char_to_level(char c)
{
  switch (c) {
    case 'E': return level::error;
    case 'W': return level::warn;
    case 'D':
    case 'V': return level::debug;
    default:  return level::info;
  }
}

// Handles both native ESP-IDF lines ("E (123) tag: msg") and the arduino-esp32
// log format ("[ 123][E][file:line] func(): msg").
static level detect_level(const char *s)
{
  if (s[0] && s[1] == ' ' && strchr("EWIDV", s[0])) {
    return char_to_level(s[0]); // native ESP-IDF
  }
  for (size_t i = 0; s[i] && i < 24; i++) { // arduino "][X]" marker
    if (s[i] == '[' && s[i + 1] && s[i + 2] == ']' && strchr("EWIDV", s[i + 1])) {
      return char_to_level(s[i + 1]);
    }
  }
  return level::info;
}

static int esp_log_vprintf(const char *fmt, va_list args)
{
  char raw[256];
  int n = vsnprintf(raw, sizeof(raw), fmt, args);

  // Preserve the exact ESP output on Serial (we replaced the default sink).
  Serial.print(raw);

  char clean[LINE_LEN];
  strip_ansi_nl(raw, clean, sizeof(clean));
  if (clean[0] == '\0') {
    return n; // nothing but newline/escape
  }

  store(detect_level(clean), clean, /*echo_serial=*/false);
  return n;
}

// ---------------------------------------------------------------------------

void init()
{
  if (s_inited) {
    return;
  }
  s_inited = true;

  if (sdcard::is_mounted()) {
    sdcard::mkdir(LOG_DIR); // no-op if it already exists
    s_file_index = next_file_index();
    open_new_file();
  }

  esp_log_set_vprintf(esp_log_vprintf);
}

void vlogf(level lvl, const char *tag, const char *fmt, va_list ap)
{
  // [mm:ss.mmm] L tag: message
  uint32_t ms = millis();
  uint32_t total_s = ms / 1000;
  uint32_t mm = total_s / 60;
  uint32_t ss = total_s % 60;
  uint32_t mmm = ms % 1000;
  const char lvl_ch[] = {'D', 'I', 'W', 'E'};

  char line[LINE_LEN];
  int p = snprintf(line, sizeof(line), "[%02u:%02u.%03u] %c %s: ",
                   (unsigned)mm, (unsigned)ss, (unsigned)mmm,
                   lvl_ch[(uint8_t)lvl], tag ? tag : "");
  if (p < 0) p = 0;
  if ((size_t)p < sizeof(line)) {
    vsnprintf(line + p, sizeof(line) - p, fmt, ap);
  }

  store(lvl, line, /*echo_serial=*/true);
}

void logf(level lvl, const char *tag, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vlogf(lvl, tag, fmt, ap);
  va_end(ap);
}

size_t line_count()
{
  return s_held;
}

const char *line_at(size_t i)
{
  if (i >= s_held) {
    return nullptr;
  }
  size_t start = (s_head + RING_LINES - s_held) % RING_LINES;
  return s_lines[(start + i) % RING_LINES];
}

level level_at(size_t i)
{
  if (i >= s_held) {
    return level::info;
  }
  size_t start = (s_head + RING_LINES - s_held) % RING_LINES;
  return (level)s_levels[(start + i) % RING_LINES];
}

uint32_t revision()
{
  return s_revision;
}

void clear()
{
  s_head = 0;
  s_held = 0;
  s_revision++;
}

bool sd_active()
{
  return s_file_open;
}

const char *current_file()
{
  return s_file_path;
}

} // namespace logging
