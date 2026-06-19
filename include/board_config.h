#pragma once

// Backlight
#define GFX_BL 6

// QSPI display bus
#define LCD_QSPI_CS   12
#define LCD_QSPI_CLK  5
#define LCD_QSPI_D0   1
#define LCD_QSPI_D1   2
#define LCD_QSPI_D2   3
#define LCD_QSPI_D3   4

// I2C (touch + TCA9554 IO-expander)
#define I2C_SDA       8
#define I2C_SCL       7

// Panel
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480

// MicroSD card — SD_MMC (SDIO) bus, 1-bit mode
#define SD_MMC_CLK_PIN 11
#define SD_MMC_CMD_PIN 10
#define SD_MMC_D0_PIN  9

// Serial Monitor app — hardware UART (Serial1) on the exposed UART header
#define UART_RX_PIN 44
#define UART_TX_PIN 43
