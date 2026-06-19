#include "apps/i2c_spi_scanner/i2c_spi_scanner.h"

#include <Arduino.h>
#include <Wire.h>

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"

// I2C addresses are 7-bit: 0x03-0x77 (0x00-0x02 and 0x78-0x7F are reserved).
static constexpr uint8_t kAddrMin = 0x03;
static constexpr uint8_t kAddrMax = 0x77;

static bool s_found[128];
static lv_obj_t *s_grid_cells[128];

struct known_device_t {
  uint8_t address;
  const char *name;
  const char *kind;
};

// Common addresses for sensors/displays seen on hobbyist I2C buses.
static const known_device_t kKnownDevices[] = {
    {0x3C, "SSD1306", "OLED"},
    {0x3D, "SSD1306", "OLED"},
    {0x68, "MPU6050", "IMU"},
    {0x69, "MPU6050", "IMU"},
    {0x76, "BME280", "Env"},
    {0x77, "BME280", "Env"},
    {0x20, "TCA9554", "IO Exp"},
    {0x3B, "AXS15231B", "Touch"},
};

static const known_device_t *lookup_known(uint8_t addr)
{
  for (const auto &d : kKnownDevices) {
    if (d.address == addr) {
      return &d;
    }
  }
  return nullptr;
}

// Builds one device row card (address / name / type tag), matching the EEZ
// mockup row visuals.
static void make_device_row(lv_obj_t *parent, uint8_t addr)
{
  const known_device_t *known = lookup_known(addr);

  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_size(row, 304, 34);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_style_bg_color(row, styles::bg_card(), 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(row, 8, 0);
  lv_obj_set_style_border_color(row, styles::border(), 0);
  lv_obj_set_style_border_width(row, 1, 0);
  lv_obj_set_style_pad_all(row, 0, 0);

  lv_obj_t *addr_lbl = lv_label_create(row);
  lv_label_set_text_fmt(addr_lbl, "0x%02X", addr);
  lv_obj_set_pos(addr_lbl, 12, 11);
  lv_obj_set_style_text_color(addr_lbl, styles::accent_green(), 0);

  lv_obj_t *name_lbl = lv_label_create(row);
  lv_label_set_text(name_lbl, known ? known->name : "Unknown device");
  lv_obj_set_pos(name_lbl, 80, 11);
  lv_obj_set_style_text_color(name_lbl, styles::text_primary(), 0);

  lv_obj_t *kind_lbl = lv_label_create(row);
  lv_label_set_text(kind_lbl, known ? known->kind : "?");
  lv_obj_set_width(kind_lbl, 64);
  lv_obj_set_pos(kind_lbl, 232, 11);
  lv_obj_set_style_text_color(kind_lbl, styles::text_muted(), 0);
  lv_obj_set_style_text_align(kind_lbl, LV_TEXT_ALIGN_RIGHT, 0);
}

// Probes every I2C address in range and rebuilds the device list + grid.
static void run_scan()
{
  uint16_t count = 0;
  for (int addr = 0; addr < 128; addr++) {
    s_found[addr] = false;
  }

  for (uint8_t addr = kAddrMin; addr <= kAddrMax; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      s_found[addr] = true;
      count++;
    }
  }

  lv_label_set_text_fmt(objects.found_count_label, "FOUND - %u DEVICE%s", count,
                         count == 1 ? "" : "S");

  lv_obj_clean(objects.device_list);
  if (count == 0) {
    lv_obj_t *l = lv_label_create(objects.device_list);
    lv_label_set_text(l, "No devices found");
    lv_obj_set_style_text_color(l, styles::text_muted(), 0);
  } else {
    for (uint8_t addr = kAddrMin; addr <= kAddrMax; addr++) {
      if (s_found[addr]) {
        make_device_row(objects.device_list, addr);
      }
    }
  }

  for (int addr = 0; addr < 128; addr++) {
    lv_obj_t *cell = s_grid_cells[addr];
    if (!cell) {
      continue;
    }
    lv_obj_set_style_bg_color(cell, s_found[addr] ? styles::accent_green() : styles::bg_surface(), 0);
  }
}

static void scan_event_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  run_scan();
}

// Builds the 16x8 address-map grid (0x00-0x7F) into objects.addr_grid.
static void build_addr_grid()
{
  lv_obj_clean(objects.addr_grid);
  lv_obj_set_flex_flow(objects.addr_grid, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(objects.addr_grid, 4, 0);
  lv_obj_set_style_pad_gap(objects.addr_grid, 2, 0);

  // 16 columns x 8 rows (rows = high nibble 0x-7x, columns = low nibble 0-F).
  // 304px wide minus padding/gaps -> ~15px cells, with a row-label column.
  const int cell_size = 15;

  // Column header row: blank spacer (aligns with row labels) + 0-F headers.
  lv_obj_t *header_row = lv_obj_create(objects.addr_grid);
  lv_obj_remove_flag(header_row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(header_row, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(header_row, LV_PCT(100), cell_size);
  lv_obj_set_style_bg_opa(header_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(header_row, 0, 0);
  lv_obj_set_style_pad_all(header_row, 0, 0);
  lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_gap(header_row, 2, 0);

  for (int col = 0; col < 16; col++) {
    lv_obj_t *col_label = lv_label_create(header_row);
    lv_label_set_text_fmt(col_label, "%X", col);
    lv_obj_set_size(col_label, cell_size, cell_size);
    lv_obj_set_style_text_align(col_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(col_label, styles::text_muted(), 0);
    lv_obj_set_style_text_font(col_label, &lv_font_montserrat_12, 0);
  }

  // Row-label spacer to align with the row-label column below.
  lv_obj_t *header_spacer = lv_label_create(header_row);
  lv_label_set_text(header_spacer, "");
  lv_obj_set_style_text_font(header_spacer, &lv_font_montserrat_12, 0);

  for (int row = 0; row < 8; row++) {
    lv_obj_t *row_obj = lv_obj_create(objects.addr_grid);
    lv_obj_remove_flag(row_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(row_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(row_obj, LV_PCT(100), cell_size);
    lv_obj_set_style_bg_opa(row_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_obj, 0, 0);
    lv_obj_set_style_pad_all(row_obj, 0, 0);
    lv_obj_set_flex_flow(row_obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(row_obj, 2, 0);

    for (int col = 0; col < 16; col++) {
      lv_obj_t *cell = lv_obj_create(row_obj);
      lv_obj_remove_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_remove_flag(cell, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_size(cell, cell_size, cell_size);
      lv_obj_set_style_radius(cell, 2, 0);
      lv_obj_set_style_border_width(cell, 0, 0);
      lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
      lv_obj_set_style_bg_color(cell, styles::bg_surface(), 0);
      s_grid_cells[row * 16 + col] = cell;
    }

    lv_obj_t *row_label = lv_label_create(row_obj);
    lv_label_set_text_fmt(row_label, "%xx", row);
    lv_obj_set_style_text_color(row_label, styles::text_muted(), 0);
    lv_obj_set_style_text_font(row_label, &lv_font_montserrat_12, 0);
  }
}

lv_obj_t *i2c_spi_scanner_create()
{
  create_screen_i2c_spi_scanner();

  lv_obj_add_style(objects.i2c_spi_scanner, &styles::style_screen, 0);

  lv_obj_add_flag(objects.scanner_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(
      objects.scanner_back_btn,
      [](lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); },
      LV_EVENT_CLICKED, NULL);

  // Only I2C scanning is implemented; SPI tab is a visual placeholder.
  lv_obj_add_flag(objects.scanner_scan_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.scanner_scan_btn, scan_event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_set_flex_flow(objects.device_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(objects.device_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(objects.device_list, 8, 0);
  lv_obj_clean(objects.device_list);

  build_addr_grid();

  lv_label_set_text(objects.found_count_label, "TAP SCAN");

  return objects.i2c_spi_scanner;
}
