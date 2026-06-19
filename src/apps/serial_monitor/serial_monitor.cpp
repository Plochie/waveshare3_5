#include "apps/serial_monitor/serial_monitor.h"

#include <Arduino.h>

#include "board_config.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"

// Show at most this many of the most recent lines (bounds LVGL object count).
static constexpr size_t MAX_VISIBLE = 120;
// Flush an in-progress line if it grows this long without a newline.
static constexpr size_t MAX_LINE_LEN = 256;

// --- Config tables (tappable cyclers) ---
static const uint32_t kBauds[] = {9600, 19200, 38400, 57600, 115200, 230400};
static const char *kBaudLabels[] = {"9600", "19200", "38400", "57600", "115200", "230400"};
static constexpr size_t kBaudCount = sizeof(kBauds) / sizeof(kBauds[0]);

static const uint32_t kFramings[] = {SERIAL_8N1, SERIAL_8E1, SERIAL_8O1, SERIAL_7E1};
static const char *kFramingLabels[] = {"8N1", "8E1", "8O1", "7E1"};
static constexpr size_t kFramingCount = sizeof(kFramings) / sizeof(kFramings[0]);

static const char *kLineEndings[] = {"\r\n", "\r", "\n", ""};
static const char *kLineEndLabels[] = {"CR+LF", "CR", "LF", "none"};
static constexpr size_t kLineEndCount = sizeof(kLineEndings) / sizeof(kLineEndings[0]);

static size_t s_baud_idx = 4;     // 115200
static size_t s_framing_idx = 0;  // 8N1
static size_t s_lineend_idx = 0;  // CR+LF

static lv_obj_t *s_scroll = nullptr;  // == objects.term_output (flex column)
static lv_timer_t *s_poll_timer = nullptr;
static String s_accum;

// Default (keyboard-hidden) Y positions of the input row, captured at create.
static lv_coord_t s_input_y = 360;
static lv_coord_t s_send_y = 366;

static void apply_serial_config()
{
  Serial1.end();
  Serial1.begin(kBauds[s_baud_idx], kFramings[s_framing_idx], UART_RX_PIN, UART_TX_PIN);
}

// Severity heuristic: color a line by simple keyword matching on its text.
static lv_color_t line_color(const char *t)
{
  if (strstr(t, "ERR") || strstr(t, "FAIL") || strstr(t, "error")) return styles::accent_red();
  if (strstr(t, "WARN") || strstr(t, "warn")) return styles::accent_amber();
  if (strstr(t, "OK") || strstr(t, "ready") || strstr(t, "connected")) return styles::accent_green();
  if (t[0] == '>') return styles::accent_blue();
  return styles::text_primary();
}

// Appends one line label to the terminal, trims the oldest beyond MAX_VISIBLE,
// and scrolls to the bottom.
static void push_line(const char *text)
{
  if (!s_scroll) return;

  lv_obj_t *l = lv_label_create(s_scroll);
  lv_obj_set_width(l, lv_pct(100));
  lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(l, line_color(text), 0);
  lv_label_set_text(l, text);

  while (lv_obj_get_child_count(s_scroll) > MAX_VISIBLE) {
    lv_obj_delete(lv_obj_get_child(s_scroll, 0));
  }

  lv_obj_scroll_to_y(s_scroll, LV_COORD_MAX / 2, LV_ANIM_OFF);
}

static void flash_dot(lv_obj_t *dot, lv_color_t color)
{
  if (dot) lv_obj_set_style_bg_color(dot, color, 0);
}

// Sends the given text + current line-ending over Serial1, echoes it locally.
static void send_text(const char *text)
{
  if (!text || !text[0]) return;
  Serial1.print(text);
  Serial1.print(kLineEndings[s_lineend_idx]);
  flash_dot(objects.tx_dot, styles::accent_amber());

  String echo = "> ";
  echo += text;
  push_line(echo.c_str());
}

// --- Polling: drain Serial1 into line labels ---
static void poll_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  bool got = false;
  while (Serial1.available()) {
    char ch = (char)Serial1.read();
    got = true;
    if (ch == '\r') continue;
    if (ch == '\n') {
      push_line(s_accum.c_str());
      s_accum = "";
    } else {
      s_accum += ch;
      if (s_accum.length() >= MAX_LINE_LEN) {
        push_line(s_accum.c_str());
        s_accum = "";
      }
    }
  }
  flash_dot(objects.rx_dot, got ? styles::accent_green() : styles::bg_surface());
}

// --- Config cyclers ---
static void baud_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  s_baud_idx = (s_baud_idx + 1) % kBaudCount;
  lv_label_set_text(objects.baud_btn, kBaudLabels[s_baud_idx]);
  apply_serial_config();
}

static void framing_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  s_framing_idx = (s_framing_idx + 1) % kFramingCount;
  lv_label_set_text(objects.framing_btn, kFramingLabels[s_framing_idx]);
  apply_serial_config();
}

static void lineend_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  s_lineend_idx = (s_lineend_idx + 1) % kLineEndCount;
  lv_label_set_text(objects.lineend_btn, kLineEndLabels[s_lineend_idx]);
}

// --- Input / keyboard ---
static void show_keyboard(bool show)
{
  if (show) {
    lv_obj_clear_flag(objects.serial_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(objects.serial_keyboard);
    // Raise the input row above the keyboard so the user sees what they type.
    lv_obj_set_y(objects.term_input, 313);
    lv_obj_set_y(objects.send_btn, 319);
  } else {
    lv_obj_add_flag(objects.serial_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_y(objects.term_input, s_input_y);
    lv_obj_set_y(objects.send_btn, s_send_y);
  }
}

static void input_clicked_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  show_keyboard(true);
}

static void send_current()
{
  const char *text = lv_textarea_get_text(objects.term_input);
  send_text(text);
  lv_textarea_set_text(objects.term_input, "");
}

static void send_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  send_current();
}

static void kb_ready_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  send_current();
  show_keyboard(false);
}

static void kb_cancel_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  show_keyboard(false);
}

// --- Macros ---
static void macro_at_cb(lv_event_t *e)   { LV_UNUSED(e); send_text("AT"); }
static void macro_rst_cb(lv_event_t *e)  { LV_UNUSED(e); send_text("AT+RST"); }
static void macro_help_cb(lv_event_t *e) { LV_UNUSED(e); send_text("help"); }

static void macro_clr_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_scroll) lv_obj_clean(s_scroll);
}

static void back_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_poll_timer) {
    lv_timer_delete(s_poll_timer);
    s_poll_timer = nullptr;
  }
  Serial1.end();
  s_scroll = nullptr;
  s_accum = "";
}

// Makes an EEZ label tappable and attaches a CLICKED handler (the scan_btn
// pattern used across the other apps).
static void on_click(lv_obj_t *obj, lv_event_cb_t cb)
{
  lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *serial_monitor_create()
{
  create_screen_serial_monitor();

  lv_obj_add_style(objects.serial_monitor, &styles::style_screen, 0);
  lv_obj_add_event_cb(objects.serial_monitor, screen_delete_cb, LV_EVENT_DELETE, NULL);

  on_click(objects.serial_back_btn, back_cb);

  // Config bar labels reflect current state; tapping cycles them.
  lv_label_set_text(objects.baud_btn, kBaudLabels[s_baud_idx]);
  lv_label_set_text(objects.framing_btn, kFramingLabels[s_framing_idx]);
  lv_label_set_text(objects.lineend_btn, kLineEndLabels[s_lineend_idx]);
  on_click(objects.baud_btn, baud_cb);
  on_click(objects.framing_btn, framing_cb);
  on_click(objects.lineend_btn, lineend_cb);

  // Terminal: turn the EEZ container into a scrolling flex column, drop the
  // design-time mockup lines.
  s_scroll = objects.term_output;
  lv_obj_set_flex_flow(s_scroll, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(s_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(s_scroll, 2, 0);
  lv_obj_clean(s_scroll);
  s_accum = "";

  // Input row + on-screen keyboard.
  s_input_y = lv_obj_get_y(objects.term_input);
  s_send_y = lv_obj_get_y(objects.send_btn);
  lv_textarea_set_one_line(objects.term_input, true);
  lv_keyboard_set_textarea(objects.serial_keyboard, objects.term_input);
  lv_obj_add_flag(objects.serial_keyboard, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(objects.term_input, input_clicked_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(objects.serial_keyboard, kb_ready_cb, LV_EVENT_READY, NULL);
  lv_obj_add_event_cb(objects.serial_keyboard, kb_cancel_cb, LV_EVENT_CANCEL, NULL);
  on_click(objects.send_btn, send_cb);

  // Macros.
  on_click(objects.macro_at_btn, macro_at_cb);
  on_click(objects.macro_rst_btn, macro_rst_cb);
  on_click(objects.macro_help_btn, macro_help_cb);
  on_click(objects.macro_clr_btn, macro_clr_cb);
  // macro_add_btn ("+ macro") is a placeholder for future user-defined macros.

  apply_serial_config();

  if (!s_poll_timer) {
    s_poll_timer = lv_timer_create(poll_cb, 50, NULL);
  }

  return objects.serial_monitor;
}
