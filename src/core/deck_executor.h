#pragma once

#include "core/deck_config.h"

// Runs a button's step list on a background FreeRTOS task so blocking HTTP and
// delay steps never stall LVGL. Phase 1 executes only direct-network steps
// (http_request / ha_service / ha_webhook / delay); other step types are
// logged and skipped.
namespace deck_executor {

struct status {
  bool active;        // a sequence is currently running
  bool ok;            // result of the last finished sequence
  char message[64];   // short human-readable last-result message
};

// Starts running btn.steps. No-op (logged) if a run is already in progress.
// Copies everything it needs, so cfg/btn may change afterwards.
void run(const deck_config::config &cfg, const deck_config::button &btn);

bool busy();

// Snapshot of the current/last run status, safe to read from the LVGL thread.
status last_status();

} // namespace deck_executor
