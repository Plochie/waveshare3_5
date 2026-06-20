#pragma once

#include <lvgl.h>

// Full-screen pairing prompt: shows the device-generated 6-digit code and
// instructs the operator to confirm it on the desktop app. Polls
// deck_client::pairing_state(); on success/rejection it shows the result
// briefly, then pops itself (calling deck_client::clear_pairing() first).
// Pushed by main.cpp when pairing becomes pending; returns a fresh screen for
// screen_manager::push().
lv_obj_t *deck_pairing_create();
