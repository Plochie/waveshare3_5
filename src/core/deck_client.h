#pragma once

#include <Arduino.h>

// Background WebSocket client for the WiFi Stream Deck: browses mDNS for the
// desktop configurator's advertised _deckhost._tcp service, connects, and
// runs the hello/auth/config-push handshake. Pushed configs are persisted to
// /deck/config.json on SD. See
// docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md.
namespace deck_client {

// Starts the background task (idempotent). Call once, after WiFi init, from
// setup() — runs independently of whether the Deck app screen is open.
void start();

// True once the current connection has completed the hello/auth_ok handshake.
bool connected();

// Returns true exactly once per config pushed by the desktop app (and clears
// the flag). The Deck app polls this from the LVGL thread to know when to
// reload /deck/config.json and rebuild its grid.
bool consume_config_pushed();

// Returns true once an icon transfer has completed and cleared the flag.
// The Deck app polls this to know when to re-render tiles that reference a
// newly-received icon.
bool consume_icons_pushed();

// Sends {"t":"exec","id":<auto>,"step":<step_json>} (protocol §3.4) and
// blocks the calling task until the matching `result` arrives or
// `timeout_ms` elapses. Safe to call from deck_executor's worker task (never
// from the LVGL thread - it blocks). Only one exec can be in flight at a
// time, which holds since deck_executor runs a button's steps sequentially.
// Returns true on {"ok":true}; otherwise false with `err` set to the
// failure reason ("not connected", "timeout", or the agent's error string).
bool exec_host_step(const String &step_json, uint32_t timeout_ms, String &err);

// Pairing handshake state, polled from the LVGL thread to drive the on-device
// pairing screen (see apps/deck/deck_pairing). `code` is the 6-digit code shown
// on the device and sent to the desktop for visual confirmation.
enum pairing_state_t { PAIR_IDLE = 0, PAIR_PENDING, PAIR_SUCCESS, PAIR_REJECTED };

struct pairing_status {
  pairing_state_t state;
  char code[7];
};

pairing_status pairing_state();

// Resets pairing state to PAIR_IDLE. Called by the pairing screen once it has
// shown the success/rejected result and is about to pop itself.
void clear_pairing();

} // namespace deck_client
