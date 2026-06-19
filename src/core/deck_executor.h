#pragma once

#include "core/deck_config.h"

// Runs a button's step list on a background FreeRTOS task so blocking HTTP,
// delay, and exec round-trips never stall LVGL. Direct-network steps
// (http_request / ha_service / ha_webhook) run locally; host steps (hotkey /
// type_text / launch_app / run_command / media_key) are sent to the desktop
// agent over deck_client::exec_host_step() (protocol §3.4). Unknown step
// types are logged and skipped.
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
