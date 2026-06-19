#pragma once

#include <lvgl.h>

// Creates the Stream Deck screen: a config-driven grid of button tiles loaded
// from /deck/config.json on SD. Tapping a tile runs its step workflow (direct-
// network actions) or navigates to another page (folder buttons).
lv_obj_t *deck_create();

// Drops the cached config so the next deck_create() reloads it from SD; if
// the Deck screen is currently open, reloads and rebuilds it immediately.
// Call from the LVGL thread only (e.g. main.cpp's loop(), after observing
// deck_client::consume_config_pushed()) — never from a background task.
void deck_invalidate_config();
