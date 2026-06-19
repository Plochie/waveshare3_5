#include "apps/app_registry.h"

#include "apps/demo/demo.h"
#include "apps/eez_demo/eez_demo.h"
#include "apps/logs/logs.h"
#include "apps/file_explorer/file_explorer.h"
#include "apps/i2c_spi_scanner/i2c_spi_scanner.h"
#include "apps/serial_monitor/serial_monitor.h"
#include "apps/wled/wled.h"

namespace app_registry {

static lv_obj_t *file_explorer_root_create()
{
  return file_explorer_create();
}

static const app_descriptor_t apps[] = {
    {"Demo", "", app_category::instruments, demo_create},
    {"EEZ Demo", "", app_category::instruments, eez_demo_create},
    {"Logs", "", app_category::debug, logs_create},
    {"Files", "", app_category::debug, file_explorer_root_create},
    {"I2C/SPI Scanner", "", app_category::instruments, i2c_spi_scanner_create},
    {"Serial Monitor", "", app_category::debug, serial_monitor_create},
    {"WLED", "", app_category::network, wled_create},
};

const app_descriptor_t *all(size_t &count)
{
  count = sizeof(apps) / sizeof(apps[0]);
  return apps;
}

} // namespace app_registry
