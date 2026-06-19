#include "apps/eez_demo/eez_demo.h"

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "apps/wifi_manager/wifi_manager.h"
#include "apps/settings/settings.h"
#include "apps/logs/logs.h"
#include "apps/file_explorer/file_explorer.h"
#include "apps/i2c_spi_scanner/i2c_spi_scanner.h"
#include "apps/serial_monitor/serial_monitor.h"
#include "apps/wled/wled.h"

static void open_wifi_manager_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(wifi_manager_create());
}

static void open_settings_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(settings_create());
}

static void open_logs_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(logs_create());
}

static void open_file_explorer_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(file_explorer_create());
}

static void open_i2c_spi_scanner_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(i2c_spi_scanner_create());
}

static void open_serial_monitor_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(serial_monitor_create());
}

static void open_wled_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  screen_manager::push(wled_create());
}

static lv_obj_t *create_nav_button(lv_obj_t *parent, const char *text, int y, lv_event_cb_t cb)
{
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 280, 50);
  lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_center(label);

  return btn;
}

lv_obj_t *eez_demo_create()
{
  // Builds a fresh screen object each call and stores it in objects.main;
  // safe to call repeatedly (e.g. re-opening the app after screen_manager
  // deleted the previous instance on pop).
  create_screen_main();

  // EEZ Studio's "dark mode" is only the editor theme - the exported screen
  // has no background color of its own, so apply our app-wide dark style.
  lv_obj_add_style(objects.main, &styles::style_screen, 0);

  create_nav_button(objects.main, "WiFi Manager", 50, open_wifi_manager_cb);
  create_nav_button(objects.main, "Settings", 110, open_settings_cb);
  create_nav_button(objects.main, "Logs", 170, open_logs_cb);
  create_nav_button(objects.main, "Files", 230, open_file_explorer_cb);
  create_nav_button(objects.main, "I2C/SPI Scanner", 290, open_i2c_spi_scanner_cb);
  create_nav_button(objects.main, "Serial Monitor", 350, open_serial_monitor_cb);
  create_nav_button(objects.main, "WLED", 410, open_wled_cb);

  return objects.main;
}
