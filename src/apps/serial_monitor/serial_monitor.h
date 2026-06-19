#pragma once

#include <lvgl.h>

// Creates the Serial Monitor screen (src/ui/eez_export). Monitors and talks
// to an external device over Serial1 (UART_RX_PIN/UART_TX_PIN), rendering
// incoming lines as a color-coded scrolling terminal with an on-screen
// keyboard and quick-send macro buttons.
lv_obj_t *serial_monitor_create();
