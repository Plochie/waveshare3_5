#pragma once

#include <lvgl.h>

// Minimal push/pop screen stack on top of lv_scr_load. Screens popped off
// the stack are deleted (lv_obj_delete) to keep RAM usage bounded.
namespace screen_manager {

void init();

// Loads `screen` and pushes it onto the navigation stack.
void push(lv_obj_t *screen);

// Returns to the previous screen and deletes the one being left.
// No-op if the stack only contains the root screen.
void pop();

} // namespace screen_manager
