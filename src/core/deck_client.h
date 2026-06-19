#pragma once

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

} // namespace deck_client
