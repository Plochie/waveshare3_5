#pragma once

#include <lvgl.h>

namespace styles {

// Dark theme palette (see esp32-summary.md  2)
inline lv_color_t bg_primary()    { return lv_color_hex(0x000000); }
inline lv_color_t bg_surface()    { return lv_color_hex(0x1A1A1F); }
inline lv_color_t bg_card()       { return lv_color_hex(0x111115); }
inline lv_color_t border()        { return lv_color_hex(0x2A2A2E); }
inline lv_color_t border_subtle() { return lv_color_hex(0x1E1E22); }

inline lv_color_t text_primary()   { return lv_color_hex(0xF0F0F0); }
inline lv_color_t text_secondary() { return lv_color_hex(0x999999); }
inline lv_color_t text_muted()     { return lv_color_hex(0x555555); }

// Category accent colors
inline lv_color_t accent_green()  { return lv_color_hex(0x3ECF8E); } // Instruments
inline lv_color_t accent_blue()   { return lv_color_hex(0x60A5FA); } // Network
inline lv_color_t accent_amber()  { return lv_color_hex(0xF59E0B); } // Debug
inline lv_color_t accent_purple() { return lv_color_hex(0xA78BFA); } // Motion
inline lv_color_t accent_red()    { return lv_color_hex(0xF87171); } // Media & Info
inline lv_color_t accent_teal()   { return lv_color_hex(0x2DD4BF); } // Camera

// Shared lv_style_t objects (screen background, cards, etc).
// Call once after lv_init().
void init();

extern lv_style_t style_screen;
extern lv_style_t style_card;

} // namespace styles
