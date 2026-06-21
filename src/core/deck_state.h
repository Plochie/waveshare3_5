#pragma once

#include <Arduino.h>

// In-memory cache of the desktop's live key/value bus (protocol §3.5). Written
// from the deck_client WS task (inbound state/state_snapshot) and read from the
// LVGL thread (tile rendering), so all access is mutex-guarded.
namespace deck_state {

// Creates the guard mutex. Call once from setup(), before deck_client::start().
void init();

// Applies an inbound value (from `state`/`state_snapshot`); marks changed.
void set_local(const String &key, const String &value);

// Current value for a key, or "" if unknown.
String get(const String &key);

// True if the key has a cached value.
bool has(const String &key);

// Updates the cache locally and sends a `set` to the desktop (toggle tap).
void set_and_send(const String &key, const String &value);

// True once per change since the last call (LVGL thread polls this to re-render).
bool consume_changed();

} // namespace deck_state
