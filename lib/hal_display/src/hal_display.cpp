#include "hal_display.h"

#include <Arduino.h>
#include <Wire.h>
#include "TCA9554.h"

#include "board_config.h"

static TCA9554 TCA(0x20);

static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_QSPI_CS, LCD_QSPI_CLK, LCD_QSPI_D0, LCD_QSPI_D1, LCD_QSPI_D2, LCD_QSPI_D3);

static Arduino_GFX *gfx = new Arduino_AXS15231B(
    bus, -1 /* RST */, 0 /* rotation */, false, SCREEN_WIDTH, SCREEN_HEIGHT);

Arduino_GFX *hal_display_init()
{
  // LCD reset is wired through the TCA9554 IO-expander, not a direct GPIO.
  TCA.begin();
  TCA.pinMode1(1, OUTPUT);
  TCA.write1(1, 1);
  delay(10);
  TCA.write1(1, 0);
  delay(10);
  TCA.write1(1, 1);
  delay(200);

  if (!gfx->begin())
  {
    return nullptr;
  }
  gfx->fillScreen(RGB565_BLACK);
  return gfx;
}

void hal_display_backlight_init()
{
  ledcAttach(GFX_BL, 5000 /* Hz */, 8 /* bit resolution */);
  ledcWrite(GFX_BL, 255);
}

void hal_display_set_brightness(uint8_t pct)
{
  if (pct > 100) {
    pct = 100;
  }
  ledcWrite(GFX_BL, (uint32_t)pct * 255 / 100);
}
