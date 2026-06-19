#pragma once

#include <lvgl.h>

// Creates the Stream Deck screen: a config-driven grid of button tiles loaded
// from /deck/config.json on SD. Tapping a tile runs its step workflow (direct-
// network actions) or navigates to another page (folder buttons).
lv_obj_t *deck_create();
