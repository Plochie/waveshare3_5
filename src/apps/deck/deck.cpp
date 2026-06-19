#include "apps/deck/deck.h"

#include <vector>

#include "core/deck_client.h"
#include "core/deck_config.h"
#include "core/deck_executor.h"
#include "core/deck_icons.h"
#include "core/screen_manager.h"
#include "ui/styles.h"

static deck_config::config s_cfg;
static bool s_loaded = false;
static std::vector<String> s_page_stack;   // navigation history of page ids
static lv_obj_t *s_root = nullptr;
static lv_obj_t *s_grid = nullptr;
static lv_obj_t *s_title = nullptr;
static lv_obj_t *s_status = nullptr;
static lv_obj_t *s_back = nullptr;
static lv_timer_t *s_status_timer = nullptr;

static const char *current_page_id()
{
  return s_page_stack.empty() ? "home" : s_page_stack.back().c_str();
}

static lv_color_t tile_color(const String &hex)
{
  if (hex.length() == 7 && hex[0] == '#') {
    long v = strtol(hex.c_str() + 1, nullptr, 16);
    return lv_color_hex((uint32_t)v);
  }
  return styles::bg_card();
}

// Forward decl.
static void rebuild_grid();

// Frees the PSRAM pixel buffer + heap-allocated lv_image_dsc_t stashed as an
// lv_image's user_data when that image (and thus the tile/grid it belonged
// to) is destroyed — otherwise every rebuild_grid()/lv_obj_clean(s_grid) call
// would leak one icon's worth of PSRAM.
static void icon_delete_cb(lv_event_t *e)
{
  lv_image_dsc_t *dsc = (lv_image_dsc_t *)lv_event_get_user_data(e);
  if (!dsc) return;
  free((void *)dsc->data);
  free(dsc);
}

// Builds an lv_image showing btn.icon (RGB565 bytes loaded from SD into a
// PSRAM buffer), scaled to fit within target_size, or returns nullptr if the
// button has no icon or the icon isn't on SD yet (label-only tile).
static lv_obj_t *make_icon(lv_obj_t *parent, const deck_config::button &btn,
                           int target_size)
{
  if (btn.icon.isEmpty()) return nullptr;
  int iw = 0, ih = 0;
  if (!deck_icons::get_dims(btn.icon, iw, ih) || iw <= 0 || ih <= 0) return nullptr;

  size_t len = 0;
  uint8_t *pixels = deck_icons::load_pixels(btn.icon, len);
  if (!pixels || len < (size_t)iw * ih * 2) {
    if (pixels) free(pixels);
    return nullptr;
  }

  lv_image_dsc_t *dsc = (lv_image_dsc_t *)malloc(sizeof(lv_image_dsc_t));
  if (!dsc) {
    free(pixels);
    return nullptr;
  }
  memset(dsc, 0, sizeof(*dsc));
  dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
  dsc->header.cf = LV_COLOR_FORMAT_RGB565;
  dsc->header.w = iw;
  dsc->header.h = ih;
  dsc->header.stride = iw * 2;
  dsc->data_size = len;
  dsc->data = pixels;

  lv_obj_t *img = lv_image_create(parent);
  lv_image_set_src(img, dsc);
  int max_src = LV_MAX(iw, ih);
  uint32_t zoom = max_src > 0 ? (uint32_t)(256 * target_size / max_src) : 256;
  lv_image_set_scale(img, zoom);
  lv_obj_add_event_cb(img, icon_delete_cb, LV_EVENT_DELETE, dsc);
  return img;
}

// Loads s_cfg from SD (if not already cached) and rebuilds the on-screen grid
// to match. Shared by deck_create() and deck_invalidate_config().
static void load_and_show()
{
  if (!s_loaded) {
    if (deck_config::load(s_cfg)) {
      s_loaded = true;
    }
  }
  s_page_stack.clear();
  s_page_stack.push_back("home");

  if (!s_loaded || s_cfg.pages.empty()) {
    lv_label_set_text(s_title, "Deck");
    lv_label_set_text(s_status, "No /deck/config.json on SD");
  } else {
    rebuild_grid();
  }
}

static void tile_click_cb(lv_event_t *e)
{
  // user_data encodes the button index within the current page.
  int idx = (int)(intptr_t)lv_event_get_user_data(e);
  const deck_config::page *pg = deck_config::find_page(s_cfg, current_page_id());
  if (!pg || idx < 0 || idx >= (int)pg->buttons.size()) return;
  const deck_config::button &btn = pg->buttons[idx];
  if (btn.open_page.length()) {
    s_page_stack.push_back(btn.open_page);
    rebuild_grid();
  } else {
    deck_executor::run(s_cfg, btn);
  }
}

static void back_click_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_page_stack.size() > 1) {
    s_page_stack.pop_back();
    rebuild_grid();
  } else {
    screen_manager::pop(); // leave the app from the home page
  }
}

static void make_tile(lv_obj_t *parent, const deck_config::button &btn, int idx,
                      int w, int h)
{
  lv_obj_t *tile = lv_obj_create(parent);
  lv_obj_set_size(tile, w, h);
  lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(tile, 10, 0);
  lv_obj_set_style_bg_color(tile, tile_color(btn.color), 0);
  lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(tile, styles::border(), 0);
  lv_obj_set_style_border_width(tile, 1, 0);
  lv_obj_set_style_pad_all(tile, 4, 0);
  lv_obj_add_flag(tile, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(tile, tile_click_cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);

  int icon_size = LV_MIN(w, h) * 6 / 10;
  lv_obj_t *icon = make_icon(tile, btn, icon_size);

  lv_obj_t *label = lv_label_create(tile);
  lv_label_set_text(label, btn.label.c_str());
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label, w - 12);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(label, styles::text_primary(), 0);
  if (icon) {
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -2);
  } else {
    lv_obj_center(label);
  }
}

static void rebuild_grid()
{
  const deck_config::page *pg = deck_config::find_page(s_cfg, current_page_id());
  lv_label_set_text(s_title, pg ? pg->title.c_str() : "Deck");
  if (s_page_stack.size() > 1) {
    lv_label_set_text(s_back, LV_SYMBOL_LEFT " Back");
  } else {
    lv_label_set_text(s_back, LV_SYMBOL_LEFT " Exit");
  }

  lv_obj_clean(s_grid);
  if (!pg) return;

  const int cols = s_cfg.cols > 0 ? s_cfg.cols : 3;
  const int rows = s_cfg.rows > 0 ? s_cfg.rows : 4;
  const int gap = 8;
  const int gw = 320 - 2 * gap;
  const int gh = 480 - 40 /*header*/ - 24 /*status*/ - gap;
  const int tw = (gw - (cols - 1) * gap) / cols;
  const int th = (gh - (rows - 1) * gap) / rows;

  for (int i = 0; i < (int)pg->buttons.size(); i++) {
    const deck_config::button &btn = pg->buttons[i];
    if (btn.pos < 0 || btn.pos >= cols * rows) continue;
    int r = btn.pos / cols;
    int c = btn.pos % cols;
    int x = c * (tw + gap);
    int y = r * (th + gap);
    lv_obj_t *holder = lv_obj_create(s_grid); // positioned wrapper
    lv_obj_remove_flag(holder, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(holder, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(holder, 0, 0);
    lv_obj_set_style_pad_all(holder, 0, 0);
    lv_obj_set_size(holder, tw, th);
    lv_obj_set_pos(holder, x, y);
    make_tile(holder, btn, i, tw, th);
  }
}

static void status_timer_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  deck_executor::status st = deck_executor::last_status();
  if (st.active) {
    lv_label_set_text(s_status, st.message);
    lv_obj_set_style_text_color(s_status, styles::accent_amber(), 0);
  } else if (st.message[0]) {
    lv_label_set_text(s_status, st.message);
    lv_obj_set_style_text_color(s_status,
        st.ok ? styles::accent_green() : styles::accent_red(), 0);
  } else {
    lv_label_set_text(s_status, deck_client::connected() ? "Agent connected" : "Agent offline");
    lv_obj_set_style_text_color(s_status, styles::text_muted(), 0);
  }
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_status_timer) {
    lv_timer_delete(s_status_timer);
    s_status_timer = nullptr;
  }
  // Pointers below are now dangling (lv_obj_delete() already ran on s_root's
  // subtree) — null them so deck_invalidate_config() can safely no-op the UI
  // half of its work if a config push arrives while the screen is closed.
  s_root = nullptr;
  s_title = nullptr;
  s_status = nullptr;
  s_grid = nullptr;
  s_back = nullptr;
}

lv_obj_t *deck_create()
{
  s_root = lv_obj_create(NULL);
  lv_obj_add_style(s_root, &styles::style_screen, 0);
  lv_obj_remove_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(s_root, 0, 0);
  lv_obj_add_event_cb(s_root, screen_delete_cb, LV_EVENT_DELETE, NULL);

  // Header: back/exit + title.
  s_back = lv_label_create(s_root);
  lv_obj_set_pos(s_back, 8, 12);
  lv_obj_set_style_text_color(s_back, styles::accent_blue(), 0);
  lv_obj_add_flag(s_back, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_back, back_click_cb, LV_EVENT_CLICKED, NULL);

  s_title = lv_label_create(s_root);
  lv_obj_set_pos(s_title, 0, 12);
  lv_obj_set_width(s_title, 320);
  lv_obj_set_style_text_align(s_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(s_title, styles::text_secondary(), 0);

  // Grid container.
  s_grid = lv_obj_create(s_root);
  lv_obj_set_pos(s_grid, 8, 40);
  lv_obj_set_size(s_grid, 320 - 16, 480 - 40 - 24);
  lv_obj_remove_flag(s_grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(s_grid, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(s_grid, 0, 0);
  lv_obj_set_style_pad_all(s_grid, 0, 0);

  // Status line.
  s_status = lv_label_create(s_root);
  lv_obj_set_pos(s_status, 8, 480 - 20);
  lv_obj_set_width(s_status, 320 - 16);
  lv_obj_set_style_text_align(s_status, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(s_status, styles::text_muted(), 0);

  // Load config once (kept in a static for re-entry).
  load_and_show();

  s_status_timer = lv_timer_create(status_timer_cb, 200, NULL);
  return s_root;
}

void deck_invalidate_config()
{
  s_loaded = false;
  if (s_root) {
    // Screen is currently open — reload + rebuild now so the change is
    // visible without requiring the user to leave and reopen the app.
    load_and_show();
  }
}
