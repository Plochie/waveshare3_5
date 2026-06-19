#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_WIFI_MANAGER = 3,
    SCREEN_ID_LOGS = 4,
    SCREEN_ID_WIFI_CONNECT = 5,
    SCREEN_ID_FILE_EXPLORER = 6,
    SCREEN_ID_FILE_VIEWER = 7,
    SCREEN_ID_I2C_SPI_SCANNER = 8,
    SCREEN_ID_SERIAL_MONITOR = 9,
    SCREEN_ID_WLED_LIGHTS = 10,
    SCREEN_ID_WLED_DETAIL = 11,
    SCREEN_ID_WLED_ADD = 12,
    SCREEN_ID_WLED_ADD_MANUAL = 13,
    _SCREEN_ID_LAST = 13
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *wifi_manager;
    lv_obj_t *logs;
    lv_obj_t *wifi_connect;
    lv_obj_t *file_explorer;
    lv_obj_t *file_viewer;
    lv_obj_t *i2c_spi_scanner;
    lv_obj_t *serial_monitor;
    lv_obj_t *wled_lights;
    lv_obj_t *wled_detail;
    lv_obj_t *wled_add;
    lv_obj_t *wled_add_manual;
    lv_obj_t *obj0;
    lv_obj_t *obj0__obj0;
    lv_obj_t *nav_wifi_btn;
    lv_obj_t *nav_settings_btn;
    lv_obj_t *nav_logs_btn;
    lv_obj_t *nav_files_btn;
    lv_obj_t *settings_back_btn;
    lv_obj_t *settings_title;
    lv_obj_t *brightness_label;
    lv_obj_t *brightness_value;
    lv_obj_t *brightness_slider;
    lv_obj_t *dim_label;
    lv_obj_t *dim_value_btn;
    lv_obj_t *wifi_back_btn;
    lv_obj_t *network_count;
    lv_obj_t *scan_btn;
    lv_obj_t *ip_label;
    lv_obj_t *network_list;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *log_back_btn;
    lv_obj_t *obj28;
    lv_obj_t *log_clear_btn;
    lv_obj_t *log_list;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *connect_back_btn;
    lv_obj_t *connect_ssid;
    lv_obj_t *pw_input;
    lv_obj_t *connect_btn;
    lv_obj_t *connect_status;
    lv_obj_t *pw_keyboard;
    lv_obj_t *fe_back_btn;
    lv_obj_t *fe_path;
    lv_obj_t *fe_refresh_btn;
    lv_obj_t *fe_list;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *obj34;
    lv_obj_t *fv_back_btn;
    lv_obj_t *fv_title;
    lv_obj_t *fv_text;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *obj37;
    lv_obj_t *scanner_back_btn;
    lv_obj_t *scanner_title;
    lv_obj_t *mode_i2c_btn;
    lv_obj_t *mode_spi_btn;
    lv_obj_t *scanner_scan_btn;
    lv_obj_t *found_count_label;
    lv_obj_t *device_list;
    lv_obj_t *obj38;
    lv_obj_t *obj39;
    lv_obj_t *obj40;
    lv_obj_t *obj41;
    lv_obj_t *obj42;
    lv_obj_t *obj43;
    lv_obj_t *obj44;
    lv_obj_t *obj45;
    lv_obj_t *obj46;
    lv_obj_t *obj47;
    lv_obj_t *obj48;
    lv_obj_t *obj49;
    lv_obj_t *addr_map_title;
    lv_obj_t *addr_grid;
    lv_obj_t *addr_hint;
    lv_obj_t *serial_back_btn;
    lv_obj_t *serial_title;
    lv_obj_t *baud_btn;
    lv_obj_t *framing_btn;
    lv_obj_t *rx_dot;
    lv_obj_t *rx_label;
    lv_obj_t *tx_dot;
    lv_obj_t *tx_label;
    lv_obj_t *lineend_btn;
    lv_obj_t *term_output;
    lv_obj_t *obj50;
    lv_obj_t *obj51;
    lv_obj_t *obj52;
    lv_obj_t *obj53;
    lv_obj_t *obj54;
    lv_obj_t *obj55;
    lv_obj_t *term_input;
    lv_obj_t *send_btn;
    lv_obj_t *macro_at_btn;
    lv_obj_t *macro_rst_btn;
    lv_obj_t *macro_help_btn;
    lv_obj_t *macro_clr_btn;
    lv_obj_t *macro_add_btn;
    lv_obj_t *serial_keyboard;
    lv_obj_t *wled_back_btn;
    lv_obj_t *wled_title;
    lv_obj_t *wled_add_btn;
    lv_obj_t *all_lights_card;
    lv_obj_t *all_lights_name;
    lv_obj_t *all_lights_summary;
    lv_obj_t *devices_label;
    lv_obj_t *wled_device_list;
    lv_obj_t *obj56;
    lv_obj_t *obj57;
    lv_obj_t *obj58;
    lv_obj_t *obj59;
    lv_obj_t *wled_hint;
    lv_obj_t *detail_back_btn;
    lv_obj_t *detail_title;
    lv_obj_t *detail_subtitle;
    lv_obj_t *live_preview;
    lv_obj_t *live_preview_label;
    lv_obj_t *hue_label;
    lv_obj_t *hue_slider;
    lv_obj_t *sat_label;
    lv_obj_t *sat_slider;
    lv_obj_t *bri_label;
    lv_obj_t *bri_slider;
    lv_obj_t *quick_label;
    lv_obj_t *quick_colors;
    lv_obj_t *solid_card;
    lv_obj_t *solid_label;
    lv_obj_t *solid_sub;
    lv_obj_t *detail_hint;
    lv_obj_t *add_back_btn;
    lv_obj_t *add_title;
    lv_obj_t *scan_section;
    lv_obj_t *scan_status;
    lv_obj_t *scan_sub;
    lv_obj_t *scan_timer;
    lv_obj_t *scan_skip_btn;
    lv_obj_t *results_section;
    lv_obj_t *results_banner;
    lv_obj_t *results_list;
    lv_obj_t *obj60;
    lv_obj_t *obj61;
    lv_obj_t *obj62;
    lv_obj_t *obj63;
    lv_obj_t *empty_section;
    lv_obj_t *empty_title;
    lv_obj_t *empty_sub;
    lv_obj_t *empty_tips;
    lv_obj_t *scan_again_btn;
    lv_obj_t *add_manual_btn;
    lv_obj_t *manual_back_btn;
    lv_obj_t *manual_title;
    lv_obj_t *manual_input_label;
    lv_obj_t *manual_input;
    lv_obj_t *manual_test_btn;
    lv_obj_t *manual_result;
    lv_obj_t *manual_result_name;
    lv_obj_t *manual_result_info;
    lv_obj_t *manual_add_btn;
    lv_obj_t *manual_keyboard;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void create_screen_wifi_manager();
void tick_screen_wifi_manager();

void create_screen_logs();
void tick_screen_logs();

void create_screen_wifi_connect();
void tick_screen_wifi_connect();

void create_screen_file_explorer();
void tick_screen_file_explorer();

void create_screen_file_viewer();
void tick_screen_file_viewer();

void create_screen_i2c_spi_scanner();
void tick_screen_i2c_spi_scanner();

void create_screen_serial_monitor();
void tick_screen_serial_monitor();

void create_screen_wled_lights();
void tick_screen_wled_lights();

void create_screen_wled_detail();
void tick_screen_wled_detail();

void create_screen_wled_add();
void tick_screen_wled_add();

void create_screen_wled_add_manual();
void tick_screen_wled_add_manual();

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_status_bar(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/