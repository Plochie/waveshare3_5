#include "apps/logs/logs.h"

#include <stdio.h>
#include <string.h>

#include "core/logging.h"
#include "core/screen_manager.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"

// Show at most this many of the most recent lines (bounds object count + cost).
static constexpr size_t MAX_VISIBLE = 120;

static lv_obj_t *s_scroll = nullptr;    // flex-column scroll container of lines
static lv_timer_t *s_refresh_timer = nullptr;
static uint32_t s_last_rev = 0xFFFFFFFF;

static lv_color_t level_color(logging::level lvl)
{
  switch (lvl) {
    case logging::level::error: return styles::accent_red();
    case logging::level::warn:  return styles::accent_amber();
    case logging::level::debug: return styles::text_muted();
    case logging::level::info:
    default:                    return styles::text_primary();
  }
}

// Renders each log line as its own label in the flex column, so every entry is
// guaranteed to be on its own line (long lines still wrap within the width).
// Also clears the EEZ design-time mockup lines on first call.
static void rebuild_text()
{
  lv_obj_clean(s_scroll);

  size_t total = logging::line_count();
  if (total == 0) {
    lv_obj_t *empty = lv_label_create(s_scroll);
    lv_label_set_text(empty, "(no log lines yet)");
    lv_obj_set_style_text_color(empty, styles::text_muted(), 0);
    return;
  }

  size_t start = (total > MAX_VISIBLE) ? (total - MAX_VISIBLE) : 0;
  for (size_t i = start; i < total; i++) {
    const char *line = logging::line_at(i);
    if (!line) continue;
    lv_obj_t *l = lv_label_create(s_scroll);
    lv_obj_set_width(l, lv_pct(100));
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(l, level_color(logging::level_at(i)), 0);
    lv_label_set_text(l, line);
  }

  // Scroll to bottom (large value is clamped to the valid range).
  lv_obj_scroll_to_y(s_scroll, LV_COORD_MAX / 2, LV_ANIM_OFF);
}

static void refresh_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  uint32_t rev = logging::revision();
  if (rev != s_last_rev) {
    s_last_rev = rev;
    rebuild_text();
  }
}

static void clear_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  logging::clear();
  s_last_rev = 0xFFFFFFFF; // force rebuild on next tick
}

static void back_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_refresh_timer) {
    lv_timer_delete(s_refresh_timer);
    s_refresh_timer = nullptr;
  }
  s_scroll = nullptr;
}

lv_obj_t *logs_create()
{
  // Build the EEZ-designed screen (header + log_list container). Rebuilds the
  // tree and reassigns objects.* each call, so it's safe to reopen after a pop.
  create_screen_logs();

  lv_obj_add_style(objects.logs, &styles::style_screen, 0);
  lv_obj_set_style_pad_all(objects.logs, 0, 0);
  lv_obj_add_event_cb(objects.logs, screen_delete_cb, LV_EVENT_DELETE, NULL);

  // Back / Clear are EEZ labels; make them tappable (the scan_btn pattern).
  lv_obj_add_flag(objects.log_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.log_back_btn, back_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(objects.log_clear_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.log_clear_btn, clear_cb, LV_EVENT_CLICKED, NULL);

  // Turn the EEZ list container into a scrolling flex column; rebuild_text()
  // drops the design-time mockup lines and fills in real log entries.
  s_scroll = objects.log_list;
  lv_obj_set_scroll_dir(s_scroll, LV_DIR_VER);
  lv_obj_set_flex_flow(s_scroll, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(s_scroll, 2, 0);

  s_last_rev = 0xFFFFFFFF;
  rebuild_text();

  s_refresh_timer = lv_timer_create(refresh_cb, 400, NULL);

  return objects.logs;
}
