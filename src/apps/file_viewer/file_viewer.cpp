#include "apps/file_viewer/file_viewer.h"

#include <string.h>

#include "hal_sdcard.h"
#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"

static void back_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::pop();
}

// Renders the file's contents as one wrapped label per line in the
// EEZ-designed fv_text container, same approach as logs.cpp's rebuild_text().
static void load_text(const char *path)
{
  lv_obj_clean(objects.fv_text);

  String content = sdcard::read_string(path);
  if (content.length() == 0) {
    lv_obj_t *empty = lv_label_create(objects.fv_text);
    lv_label_set_text(empty, "(empty file)");
    lv_obj_set_style_text_color(empty, styles::text_muted(), 0);
    return;
  }

  int pos = 0;
  while (pos <= (int)content.length()) {
    int nl = content.indexOf('\n', pos);
    String line = (nl < 0) ? content.substring(pos) : content.substring(pos, nl);

    lv_obj_t *l = lv_label_create(objects.fv_text);
    lv_obj_set_width(l, lv_pct(100));
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(l, styles::text_primary(), 0);
    lv_label_set_text(l, line.c_str());

    if (nl < 0) {
      break;
    }
    pos = nl + 1;
  }
}

lv_obj_t *file_viewer_create(const char *path)
{
  create_screen_file_viewer();
  lv_obj_add_style(objects.file_viewer, &styles::style_screen, 0);

  // Filename only (basename) for the title.
  const char *slash = path ? strrchr(path, '/') : nullptr;
  const char *filename = slash ? (slash + 1) : (path ? path : "");
  lv_label_set_text(objects.fv_title, filename[0] ? filename : "/");

  lv_obj_add_flag(objects.fv_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.fv_back_btn, back_cb, LV_EVENT_CLICKED, NULL);

  // Turn the EEZ list container into a scrolling flex column, same as
  // logs.cpp's log_list.
  lv_obj_set_scroll_dir(objects.fv_text, LV_DIR_VER);
  lv_obj_set_flex_flow(objects.fv_text, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(objects.fv_text, 2, 0);

  load_text(path);

  return objects.file_viewer;
}
