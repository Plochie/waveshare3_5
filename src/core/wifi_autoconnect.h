#pragma once

// Non-blocking boot-time auto-connect using networks saved by wifi_store.
namespace wifi_autoconnect {

// Loads known networks and starts trying them in order (most-recent first),
// each with a timeout, until one connects. No-op if none are saved. Safe to
// call once after lv_init() (uses an lv_timer to drive retries).
void start();

} // namespace wifi_autoconnect
