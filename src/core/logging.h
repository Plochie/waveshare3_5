#pragma once

#include <Arduino.h>
#include <stdarg.h>

// App-wide logging. Each line is mirrored to: (1) Serial, (2) an in-RAM ring
// buffer (for the Logs app), and (3) a rotating file under /logs on the SD
// card. ESP-IDF logs (log_e/log_i/log_w) are captured too via an esp_log hook.
//
// Use the LOG_* macros from app code. Call logging::init() once at boot, after
// the SD card has been mounted (sdcard::init()), so the SD file can be opened.
namespace logging {

enum class level : uint8_t { debug = 0, info, warn, error };

// Initialize the ring buffer, open the per-boot SD log file (if a card is
// mounted), and install the esp_log capture hook. Safe to call once.
void init();

// Core entry points (used by the LOG_* macros).
void logf(level lvl, const char *tag, const char *fmt, ...);
void vlogf(level lvl, const char *tag, const char *fmt, va_list ap);

// --- Ring-buffer access for the Logs UI ---
// Number of lines currently held (<= ring capacity).
size_t line_count();
// Line `i`, where 0 is the oldest currently held line. nullptr if out of range.
const char *line_at(size_t i);
// Level of line `i` (info if out of range).
level level_at(size_t i);
// Monotonic counter bumped on every new line; UIs poll this to detect changes.
uint32_t revision();
// Clears the in-RAM ring buffer only (does not touch the SD file).
void clear();

// --- SD status ---
bool sd_active();
const char *current_file(); // active SD log path, or "" if none

} // namespace logging

#define LOG_D(tag, ...) ::logging::logf(::logging::level::debug, tag, __VA_ARGS__)
#define LOG_I(tag, ...) ::logging::logf(::logging::level::info, tag, __VA_ARGS__)
#define LOG_W(tag, ...) ::logging::logf(::logging::level::warn, tag, __VA_ARGS__)
#define LOG_E(tag, ...) ::logging::logf(::logging::level::error, tag, __VA_ARGS__)
