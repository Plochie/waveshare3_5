#pragma once

#include <Arduino.h>
#include <vector>

// Persists Stream Deck icons (raw RGB565, protocol §3.3) under
// /deck/icons/ on SD. Mirrors the desktop's <name> + <name>.json sidecar
// scheme (desktop/src-tauri/src/icons.rs) so both sides agree on
// dimensions without re-deriving them from file size.
namespace deck_icons {

// Names of all icons currently stored on SD (the ".bin" is part of the
// name, as sent in icon_begin/icon_end/have_icons).
std::vector<String> all_names();

// Looks up the sidecar dimensions for a stored icon. Returns false if the
// icon (or its sidecar) isn't present.
bool get_dims(const String &name, int &w, int &h);

// Begins receiving a new icon transfer (icon_begin): truncates
// /deck/icons/<name>, remembers w/h for the sidecar written by
// end_receive(). Returns false if SD isn't writable.
bool begin_receive(const String &name, size_t total_bytes, int w, int h);

// Appends one WS binary frame's payload to the in-progress transfer.
// Returns false if there's no transfer in progress.
bool write_chunk(const uint8_t *data, size_t len);

// Finishes the in-progress transfer (icon_end): writes the dimensions
// sidecar. Returns false if no transfer was in progress.
bool end_receive();

// Loads an icon's raw RGB565 bytes into a PSRAM-backed buffer (caller
// frees with free()). Returns nullptr if the icon isn't on SD.
uint8_t *load_pixels(const String &name, size_t &len);

} // namespace deck_icons
