#include "apps/deck/deck_pairing.h"

#include "core/deck_client.h"
#include "core/screen_manager.h"
#include "ui/styles.h"

static lv_obj_t *s_proot = nullptr;
static lv_obj_t *s_pcode = nullptr;
static lv_obj_t *s_pstatus = nullptr;
static lv_timer_t *s_ptimer = nullptr;
static bool s_popping = false;

static void pop_now_cb(lv_timer_t *t)
{
  lv_timer_delete(t);
  deck_client::clear_pairing();
  screen_manager::pop();
}

static void schedule_pop(uint32_t delay_ms)
{
  if (s_popping) return;
  s_popping = true;
  // pop_now_cb deletes this timer itself on its first (only) firing.
  lv_timer_create(pop_now_cb, delay_ms, nullptr);
}

static void ptimer_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  deck_client::pairing_status ps = deck_client::pairing_state();
  switch (ps.state) {
    case deck_client::PAIR_PENDING:
      lv_label_set_text(s_pcode, ps.code);
      break;
    case deck_client::PAIR_SUCCESS:
      lv_label_set_text(s_pstatus, LV_SYMBOL_OK " Paired");
      lv_obj_set_style_text_color(s_pstatus, styles::accent_green(), 0);
      schedule_pop(900);
      break;
    case deck_client::PAIR_REJECTED:
      lv_label_set_text(s_pstatus, LV_SYMBOL_CLOSE " Rejected");
      lv_obj_set_style_text_color(s_pstatus, styles::accent_red(), 0);
      schedule_pop(1500);
      break;
    case deck_client::PAIR_IDLE:
    default:
      // Disconnected while pending — drop back to whatever was underneath.
      schedule_pop(0);
      break;
  }
}

static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_ptimer) {
    lv_timer_delete(s_ptimer);
    s_ptimer = nullptr;
  }
  s_proot = nullptr;
  s_pcode = nullptr;
  s_pstatus = nullptr;
  s_popping = false;
}

lv_obj_t *deck_pairing_create()
{
  s_popping = false;
  s_proot = lv_obj_create(NULL);
  lv_obj_add_style(s_proot, &styles::style_screen, 0);
  lv_obj_remove_flag(s_proot, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(s_proot, screen_delete_cb, LV_EVENT_DELETE, NULL);

  lv_obj_t *title = lv_label_create(s_proot);
  lv_label_set_text(title, "Pairing");
  lv_obj_set_style_text_color(title, styles::text_secondary(), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

  s_pcode = lv_label_create(s_proot);
  deck_client::pairing_status ps = deck_client::pairing_state();
  lv_label_set_text(s_pcode, ps.code[0] ? ps.code : "------");
  lv_obj_set_style_text_color(s_pcode, styles::text_primary(), 0);
  lv_obj_set_style_text_font(s_pcode, &lv_font_montserrat_28, 0);
  lv_obj_align(s_pcode, LV_ALIGN_CENTER, 0, -20);

  lv_obj_t *hint = lv_label_create(s_proot);
  lv_label_set_text(hint, "Confirm this code on your\ncomputer to pair this deck.");
  lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(hint, 280);
  lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(hint, styles::text_muted(), 0);
  lv_obj_align(hint, LV_ALIGN_CENTER, 0, 40);

  s_pstatus = lv_label_create(s_proot);
  lv_label_set_text(s_pstatus, "waiting...");
  lv_obj_set_style_text_color(s_pstatus, styles::text_muted(), 0);
  lv_obj_align(s_pstatus, LV_ALIGN_BOTTOM_MID, 0, -40);

  s_ptimer = lv_timer_create(ptimer_cb, 200, NULL);
  return s_proot;
}
