#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

// Pull in the arduino-esp32 "Network" lib that WiFi depends on (WiFi declares
// no `depends`), same LDF note as the wifi_manager/wifi_connect apps.
#include <Network.h>
#include <WiFi.h>

#include "board_config.h"
#include "hal_display.h"
#include "hal_sdcard.h"
#include "esp_lcd_touch_axs15231b.h"

#include "ui/styles.h"
#include "core/screen_manager.h"
#include "core/logging.h"
#include "core/wifi_autoconnect.h"
#include "core/deck_client.h"
#include "core/deck_state.h"
#include "core/display_power.h"
#include "apps/eez_demo/eez_demo.h"
#include "apps/deck/deck.h"
#include "apps/deck/deck_pairing.h"

#define DIRECT_RENDER_MODE // Uncomment to enable full frame buffer

Arduino_GFX *gfx = nullptr;

uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf1;
lv_color_t *disp_draw_buf2;

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

uint32_t millis_cb(void)
{
  return millis();
}

// Logs STA connect/disconnect events for the whole session (Serial + Logs app
// + SD), independent of which screen is active - useful to catch drops that
// happen while the WiFi Manager screen isn't open.
void wifi_event_cb(arduino_event_id_t event, arduino_event_info_t info)
{
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      LOG_W("wifi", "STA disconnected, reason=%d",
            info.wifi_sta_disconnected.reason);
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      LOG_I("wifi", "STA got ip %s", WiFi.localIP().toString().c_str());
      break;
    default:
      break;
  }
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

  /*Call it to tell LVGL you are ready*/
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  // Latches a wake tap so the whole press-hold-release is hidden from LVGL.
  // The finger is still down for many read cycles after the backlight comes
  // back, so swallowing only the first frame would let the rest of the press
  // click through.
  static bool swallow_until_release = false;

  touch_data_t touch_data;
  bsp_touch_read();
  bool pressed = bsp_touch_get_coordinates(&touch_data);

  // First tap after auto-dim only wakes the screen; don't deliver it as a
  // click. Wake immediately, then keep reporting "released" until the finger
  // lifts so no widget under the tap is activated.
  if (pressed && display_power::is_dimmed()) {
    display_power::wake();
    swallow_until_release = true;
  }
  if (swallow_until_release) {
    if (!pressed) {
      swallow_until_release = false;
    }
    data->state = LV_INDEV_STATE_REL;
    return;
  }

  if (pressed) {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touch_data.coords[0].x;
    data->point.y = touch_data.coords[0].y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void setup()
{
  // Bring up USB-CDC serial first and wait (briefly) for the host monitor to
  // re-attach after the reset, otherwise early setup() prints are dropped.
  // The timeout keeps boot fast when running headless (no monitor attached).
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {
    delay(10);
  }
  delay(200);

  // Mount the SD card and start logging before anything else, so boot
  // messages are captured to /logs and the on-device Logs app.
  bool sd_ok = sdcard::init();
  logging::init();

  LOG_I("boot", "Waveshare ESP32-S3-Touch-LCD-3.5B, LVGL v%d.%d.%d",
        lv_version_major(), lv_version_minor(), lv_version_patch());
  if (sd_ok) {
    LOG_I("sd", "card %s, %lluMB, logging to %s", sdcard::type_str(),
          (unsigned long long)(sdcard::size_bytes() / (1024 * 1024)),
          logging::current_file());
  } else {
    LOG_W("sd", "no card - logging to serial + RAM only");
  }

  WiFi.onEvent(wifi_event_cb);

  Wire.begin(I2C_SDA, I2C_SCL);

  gfx = hal_display_init();
  hal_display_backlight_init();
  if (!gfx) {
    LOG_E("display", "hal_display_init() / gfx->begin() failed");
  }

  bsp_touch_init(&Wire, -1, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(millis_cb);

  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

#ifdef DIRECT_RENDER_MODE
  bufSize = SCREEN_WIDTH * SCREEN_HEIGHT;
  disp_draw_buf1 = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
  bufSize = SCREEN_WIDTH * 40;
  disp_draw_buf1 = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#endif
  if (!disp_draw_buf1 || !disp_draw_buf2)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, my_disp_flush);
#ifdef DIRECT_RENDER_MODE
    lv_display_set_buffers(disp, disp_draw_buf1, disp_draw_buf2, bufSize * 2, LV_DISPLAY_RENDER_MODE_FULL);
#else
    lv_display_set_buffers(disp, disp_draw_buf1, disp_draw_buf2, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    /*Initialize the touchpad input device driver*/
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    styles::init();
    screen_manager::init();
    display_power::init();
    screen_manager::push(eez_demo_create());
    wifi_autoconnect::start();
    deck_state::init();
    deck_client::start();
  }

  Serial.println("Setup done");
}

void loop()
{
  lv_task_handler(); /* let the GUI do its work */
  // Runs regardless of which screen is active, so a config pushed while the
  // Deck app is closed still invalidates its cache for next time it's opened.
  if (deck_client::consume_config_pushed() || deck_client::consume_icons_pushed()) {
    deck_invalidate_config();
  }

  // Surface the pairing screen the moment the device needs to pair, over
  // whatever app is open. The screen pops itself once pairing resolves (which
  // resets state to PAIR_IDLE via deck_client::clear_pairing()).
  static bool s_pair_open = false;
  deck_client::pairing_status ps = deck_client::pairing_state();
  if (ps.state == deck_client::PAIR_PENDING && !s_pair_open) {
    screen_manager::push(deck_pairing_create());
    s_pair_open = true;
  } else if (ps.state == deck_client::PAIR_IDLE && s_pair_open) {
    s_pair_open = false;
  }
  delay(5);
}
