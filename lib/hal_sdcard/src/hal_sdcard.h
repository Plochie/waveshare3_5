#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>

// Generic MicroSD read/write driver for the Waveshare ESP32-S3-Touch-LCD-3.5B.
// The card is wired to the SD_MMC (SDIO) bus and mounted in 1-bit mode using
// the pins in board_config.h.
//
// Two layers are exposed:
//   * mount/info + small file convenience helpers (this namespace), and
//   * fs() for direct use of the standard Arduino File / fs::FS API when a
//     caller needs streaming, seeking, or partial reads.
//
// All paths are absolute and rooted at the card, e.g. "/logs/run.txt".
namespace sdcard {

// Mounts the card (idempotent). Returns true if a card is mounted and ready.
bool init();

// Unmounts the card. init() can be called again afterwards.
void deinit();

bool is_mounted();

// --- Card info (return 0 / "NONE" when no card is mounted) ---
uint64_t size_bytes();   // total physical card capacity
uint64_t total_bytes();  // total filesystem space
uint64_t used_bytes();   // used filesystem space
const char *type_str();  // "SDSC" / "SDHC" / "MMC" / "UNKNOWN" / "NONE"

// Underlying filesystem, for direct File-based access (streaming, seeking…).
fs::FS &fs();

// --- File convenience helpers ---
bool exists(const char *path);

// Create/overwrite `path` with the given contents. Parent dir must exist.
bool write(const char *path, const uint8_t *data, size_t len);
bool write(const char *path, const char *text);

// Append to `path`, creating it if missing.
bool append(const char *path, const uint8_t *data, size_t len);
bool append(const char *path, const char *text);

// Read up to `maxlen` bytes of `path` into `buf`.
// Returns the number of bytes read, or -1 if the file can't be opened.
int read(const char *path, uint8_t *buf, size_t maxlen);

// Read the whole file as a String (empty on error / empty file).
String read_string(const char *path);

bool remove(const char *path);
bool rename(const char *from, const char *to);
bool mkdir(const char *path);
bool rmdir(const char *path);

// Iterate the entries of `dir`, invoking `cb` for each. `user` is passed
// through to the callback. Returns false if `dir` can't be opened.
bool list(const char *dir,
          void (*cb)(const char *name, bool is_dir, size_t size, void *user),
          void *user = nullptr);

// Mounts (if needed) and runs a write/read/list round-trip, logging to Serial.
// Returns true if the write+read-back of a temp file succeeded.
bool self_test();

} // namespace sdcard
