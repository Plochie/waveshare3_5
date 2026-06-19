#include "apps/file_explorer/file_explorer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "hal_sdcard.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "apps/file_viewer/file_viewer.h"

// Current directory being shown; re-used by rebuild_list()/list_entry_cb()
// since this screen is re-created for every directory level.
static char s_path[256];
static int s_entry_count = 0;

static void rebuild_list();

// Extensions whose contents make sense to show in file_viewer.
static bool is_text_file(const char *name)
{
  const char *dot = strrchr(name, '.');
  if (!dot) {
    return false;
  }
  static const char *kTextExt[] = {
      ".txt", ".log", ".json", ".cfg", ".ini", ".csv",
      ".md",  ".h",   ".hpp",  ".c",   ".cpp", ".yaml", ".yml", ".conf",
  };
  for (size_t i = 0; i < sizeof(kTextExt) / sizeof(kTextExt[0]); i++) {
    if (strcasecmp(dot, kTextExt[i]) == 0) {
      return true;
    }
  }
  return false;
}

static void format_size(char *out, size_t outsz, size_t bytes)
{
  if (bytes < 1024) {
    snprintf(out, outsz, "%u B", (unsigned)bytes);
  } else if (bytes < 1024u * 1024u) {
    snprintf(out, outsz, "%.1f KB", bytes / 1024.0);
  } else {
    snprintf(out, outsz, "%.1f MB", bytes / (1024.0 * 1024.0));
  }
}

// Builds the absolute path of `name` inside the current directory `s_path`.
static void join_path(char *out, size_t outsz, const char *name)
{
  if (strcmp(s_path, "/") == 0) {
    snprintf(out, outsz, "/%s", name);
  } else {
    snprintf(out, outsz, "%s/%s", s_path, name);
  }
}

// Frees the strdup'd full path stashed in user_data when a row is destroyed
// (lv_obj_clean() during refresh, or screen teardown on pop).
static void row_delete_cb(lv_event_t *e)
{
  lv_obj_t *row = (lv_obj_t *)lv_event_get_target(e);
  free(lv_obj_get_user_data(row));
}

static void dir_click_cb(lv_event_t *e)
{
  lv_obj_t *row = (lv_obj_t *)lv_event_get_target(e);
  const char *path = (const char *)lv_obj_get_user_data(row);
  screen_manager::push(file_explorer_create(path));
}

static void file_click_cb(lv_event_t *e)
{
  lv_obj_t *row = (lv_obj_t *)lv_event_get_target(e);
  const char *path = (const char *)lv_obj_get_user_data(row);
  screen_manager::push(file_viewer_create(path));
}

static void msgbox_delete_cb(lv_event_t *e)
{
  lv_obj_t *mb = (lv_obj_t *)lv_event_get_target(e);
  free(lv_obj_get_user_data(mb));
}

static void msgbox_cancel_cb(lv_event_t *e)
{
  lv_obj_t *mb = (lv_obj_t *)lv_event_get_user_data(e);
  lv_msgbox_close(mb);
}

static void msgbox_delete_confirm_cb(lv_event_t *e)
{
  lv_obj_t *mb = (lv_obj_t *)lv_event_get_user_data(e);
  const char *path = (const char *)lv_obj_get_user_data(mb);
  sdcard::remove(path);
  lv_msgbox_close(mb);
  rebuild_list();
}

// Shows a Delete/Cancel confirmation dialog for `path` (full SD path).
static void confirm_delete(const char *path)
{
  lv_obj_t *mb = lv_msgbox_create(NULL);
  lv_msgbox_add_title(mb, "Delete file?");
  lv_msgbox_add_text(mb, path);
  lv_obj_t *cancel_btn = lv_msgbox_add_footer_button(mb, "Cancel");
  lv_obj_t *delete_btn = lv_msgbox_add_footer_button(mb, "Delete");

  lv_obj_set_user_data(mb, strdup(path));
  lv_obj_add_event_cb(mb, msgbox_delete_cb, LV_EVENT_DELETE, NULL);
  lv_obj_add_event_cb(cancel_btn, msgbox_cancel_cb, LV_EVENT_CLICKED, mb);
  lv_obj_add_event_cb(delete_btn, msgbox_delete_confirm_cb, LV_EVENT_CLICKED, mb);
}

// Tapping the trash icon opens the confirmation dialog for the row's file,
// without also triggering the row's own click handler (file_click_cb).
static void trash_click_cb(lv_event_t *e)
{
  lv_event_stop_bubbling(e);
  lv_obj_t *trash = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *row = lv_obj_get_parent(trash);
  const char *path = (const char *)lv_obj_get_user_data(row);
  confirm_delete(path);
}

// Builds one row for a directory entry. Directories are clickable (navigate
// in); text files are clickable (open file_viewer) and show a trash icon;
// other files are inert except for the trash icon.
static void list_entry_cb(const char *name, bool is_dir, size_t size, void *user)
{
  lv_obj_t *parent = (lv_obj_t *)user;
  s_entry_count++;

  char full_path[256];
  join_path(full_path, sizeof(full_path), name);

  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_size(row, 300, 36);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_style_bg_color(row, styles::bg_card(), 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(row, 4, 0);
  lv_obj_set_style_border_color(row, styles::border(), 0);
  lv_obj_set_style_border_width(row, 1, 0);
  lv_obj_set_style_pad_all(row, 0, 0);

  lv_obj_set_user_data(row, strdup(full_path));
  lv_obj_add_event_cb(row, row_delete_cb, LV_EVENT_DELETE, NULL);

  bool show_trash = !is_dir;
  int name_width = show_trash ? 180 : 280;

  lv_obj_t *name_label = lv_label_create(row);
  lv_obj_set_pos(name_label, 8, 9);
  lv_obj_set_width(name_label, name_width);
  lv_label_set_long_mode(name_label, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(name_label, is_dir ? styles::accent_amber() : styles::text_primary(), 0);
  lv_label_set_text_fmt(name_label, "%s  %s",
                        is_dir ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, name);

  if (is_dir) {
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, dir_click_cb, LV_EVENT_CLICKED, NULL);
  } else {
    char size_str[16];
    format_size(size_str, sizeof(size_str), size);
    lv_obj_t *size_label = lv_label_create(row);
    lv_obj_set_pos(size_label, 188, 9);
    lv_obj_set_width(size_label, 56);
    lv_obj_set_style_text_align(size_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(size_label, styles::text_secondary(), 0);
    lv_label_set_text(size_label, size_str);

    if (is_text_file(name)) {
      lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(row, file_click_cb, LV_EVENT_CLICKED, NULL);
    }
  }

  if (show_trash) {
    lv_obj_t *trash = lv_label_create(row);
    lv_obj_set_pos(trash, 264, 9);
    lv_obj_set_width(trash, 24);
    lv_obj_set_style_text_color(trash, styles::accent_red(), 0);
    lv_label_set_text(trash, LV_SYMBOL_TRASH);
    lv_obj_add_flag(trash, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(trash, trash_click_cb, LV_EVENT_CLICKED, NULL);
  }
}

// Replaces fe_list's contents with a single muted centered placeholder line.
static void show_placeholder(const char *msg)
{
  lv_obj_clean(objects.fe_list);
  lv_obj_t *l = lv_label_create(objects.fe_list);
  lv_obj_set_style_text_color(l, styles::text_muted(), 0);
  lv_label_set_text(l, msg);
}

static void rebuild_list()
{
  lv_obj_clean(objects.fe_list);
  lv_label_set_text(objects.fe_path, s_path);

  s_entry_count = 0;
  bool ok = sdcard::list(s_path, list_entry_cb, objects.fe_list);
  if (!ok) {
    show_placeholder("Can't open directory");
    return;
  }
  if (s_entry_count == 0) {
    show_placeholder("(empty)");
  }
}

static void back_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

static void refresh_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  rebuild_list();
}

lv_obj_t *file_explorer_create(const char *path)
{
  strncpy(s_path, (path && path[0]) ? path : "/", sizeof(s_path) - 1);
  s_path[sizeof(s_path) - 1] = '\0';

  create_screen_file_explorer();
  lv_obj_add_style(objects.file_explorer, &styles::style_screen, 0);

  lv_obj_add_flag(objects.fe_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.fe_back_btn, back_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(objects.fe_refresh_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.fe_refresh_btn, refresh_cb, LV_EVENT_CLICKED, NULL);

  // Turn the EEZ list container into a scrolling flex column and drop the
  // design-time mockup rows; populated by rebuild_list().
  lv_obj_set_scroll_dir(objects.fe_list, LV_DIR_VER);
  lv_obj_set_flex_flow(objects.fe_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(objects.fe_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(objects.fe_list, 6, 0);

  rebuild_list();

  return objects.file_explorer;
}
