#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 30);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_status_bar(obj, 14);
        }
        {
            // nav_wifi_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.nav_wifi_btn = obj;
            lv_obj_set_pos(obj, 20, 50);
            lv_obj_set_size(obj, 280, 50);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "WiFi Manager");
        }
        {
            // nav_settings_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.nav_settings_btn = obj;
            lv_obj_set_pos(obj, 20, 110);
            lv_obj_set_size(obj, 280, 50);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Settings");
        }
        {
            // nav_logs_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.nav_logs_btn = obj;
            lv_obj_set_pos(obj, 20, 170);
            lv_obj_set_size(obj, 280, 50);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Logs");
        }
        {
            // nav_files_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.nav_files_btn = obj;
            lv_obj_set_pos(obj, 20, 230);
            lv_obj_set_size(obj, 280, 50);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Files");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    tick_user_widget_status_bar(14);
}

void create_screen_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // settings_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.settings_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // settings_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.settings_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SETTINGS");
        }
        {
            // brightness_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.brightness_label = obj;
            lv_obj_set_pos(obj, 16, 40);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Brightness");
        }
        {
            // brightness_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.brightness_value = obj;
            lv_obj_set_pos(obj, 154, 40);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "100%");
        }
        {
            // brightness_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.brightness_slider = obj;
            lv_obj_set_pos(obj, 16, 64);
            lv_obj_set_size(obj, 288, 10);
            lv_slider_set_range(obj, 10, 100);
            lv_slider_set_value(obj, 100, LV_ANIM_OFF);
        }
        {
            // dim_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dim_label = obj;
            lv_obj_set_pos(obj, 16, 100);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Screen off after");
        }
        {
            // dim_value_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dim_value_btn = obj;
            lv_obj_set_pos(obj, 154, 100);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "30s");
        }
    }
    
    tick_screen_settings();
}

void tick_screen_settings() {
}

void create_screen_wifi_manager() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wifi_manager = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // wifi_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wifi_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // network_count
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.network_count = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "4 NETWORKS");
        }
        {
            // scan_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.scan_btn = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Scan");
        }
        {
            // ip_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ip_label = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Not connected");
        }
        {
            // network_list
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.network_list = obj;
            lv_obj_set_pos(obj, 0, 52);
            lv_obj_set_size(obj, LV_PCT(100), 428);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj1 = obj;
                    lv_obj_set_pos(obj, 10, 70);
                    lv_obj_set_size(obj, 300, 78);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            lv_obj_set_pos(obj, 12, 27);
                            lv_obj_set_size(obj, 26, 24);
                            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj2 = obj;
                                    lv_obj_set_pos(obj, 0, 18);
                                    lv_obj_set_size(obj, 5, 6);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj3 = obj;
                                    lv_obj_set_pos(obj, 7, 12);
                                    lv_obj_set_size(obj, 5, 12);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj4 = obj;
                                    lv_obj_set_pos(obj, 14, 6);
                                    lv_obj_set_size(obj, 5, 18);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj5 = obj;
                                    lv_obj_set_pos(obj, 21, 0);
                                    lv_obj_set_size(obj, 5, 24);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                            }
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj6 = obj;
                            lv_obj_set_pos(obj, 50, 8);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "HomeNet_5G");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj7 = obj;
                            lv_obj_set_pos(obj, 50, 32);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "ch 36 | WPA2 | 5GHz");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj8 = obj;
                            lv_obj_set_pos(obj, 228, 8);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "-42");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj9 = obj;
                            lv_obj_set_pos(obj, 50, 52);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "A4:2B:8C:1D:3E:9F");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj10 = obj;
                    lv_obj_set_pos(obj, 10, 156);
                    lv_obj_set_size(obj, 300, 78);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            lv_obj_set_pos(obj, 12, 27);
                            lv_obj_set_size(obj, 26, 24);
                            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj11 = obj;
                                    lv_obj_set_pos(obj, 0, 18);
                                    lv_obj_set_size(obj, 5, 6);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj12 = obj;
                                    lv_obj_set_pos(obj, 7, 12);
                                    lv_obj_set_size(obj, 5, 12);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj13 = obj;
                                    lv_obj_set_pos(obj, 14, 6);
                                    lv_obj_set_size(obj, 5, 18);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj14 = obj;
                                    lv_obj_set_pos(obj, 21, 0);
                                    lv_obj_set_size(obj, 5, 24);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                            }
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj15 = obj;
                            lv_obj_set_pos(obj, 50, 8);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "JIOFIBER_4F2");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj16 = obj;
                            lv_obj_set_pos(obj, 50, 32);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "ch 6 | WPA3 | 2.4GHz");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj17 = obj;
                            lv_obj_set_pos(obj, 228, 8);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "-67");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj18 = obj;
                            lv_obj_set_pos(obj, 50, 52);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "F0:9F:C2:14:5A:7B");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj19 = obj;
                    lv_obj_set_pos(obj, 10, 328);
                    lv_obj_set_size(obj, 300, 78);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            lv_obj_set_pos(obj, 12, 27);
                            lv_obj_set_size(obj, 26, 24);
                            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj20 = obj;
                                    lv_obj_set_pos(obj, 0, 18);
                                    lv_obj_set_size(obj, 5, 6);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj21 = obj;
                                    lv_obj_set_pos(obj, 7, 12);
                                    lv_obj_set_size(obj, 5, 12);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj22 = obj;
                                    lv_obj_set_pos(obj, 14, 6);
                                    lv_obj_set_size(obj, 5, 18);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj23 = obj;
                                    lv_obj_set_pos(obj, 21, 0);
                                    lv_obj_set_size(obj, 5, 24);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                            }
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj24 = obj;
                            lv_obj_set_pos(obj, 50, 8);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "AndroidAP_99");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj25 = obj;
                            lv_obj_set_pos(obj, 50, 32);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "ch 1 · WPA2 · 2.4GHz");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj26 = obj;
                            lv_obj_set_pos(obj, 228, 8);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "-88");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj27 = obj;
                            lv_obj_set_pos(obj, 50, 52);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "3C:71:BF:62:8D:44");
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_wifi_manager();
}

void tick_screen_wifi_manager() {
}

void create_screen_logs() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.logs = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // log_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.log_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "LOGS");
        }
        {
            // log_clear_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.log_clear_btn = obj;
            lv_obj_set_pos(obj, 222, 8);
            lv_obj_set_size(obj, 90, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Clear");
        }
        {
            // log_list
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.log_list = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 448);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj29 = obj;
                    lv_obj_set_pos(obj, 6, 4);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[INFO]  system boot ok");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj30 = obj;
                    lv_obj_set_pos(obj, 6, 22);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[WARN]  battery low");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj31 = obj;
                    lv_obj_set_pos(obj, 6, 40);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf87171), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[ERROR] i2c read timeout");
                }
            }
        }
    }
    
    tick_screen_logs();
}

void tick_screen_logs() {
}

void create_screen_wifi_connect() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wifi_connect = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // connect_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.connect_back_btn = obj;
            lv_obj_set_pos(obj, 8, 10);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // connect_ssid
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.connect_ssid = obj;
            lv_obj_set_pos(obj, 0, 10);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Connect to Network");
        }
        {
            // pw_input
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.pw_input = obj;
            lv_obj_set_pos(obj, 16, 44);
            lv_obj_set_size(obj, 288, 44);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "Password");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, true);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // connect_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.connect_btn = obj;
            lv_obj_set_pos(obj, 16, 98);
            lv_obj_set_size(obj, 288, 40);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 11, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 11, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Connect");
        }
        {
            // connect_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.connect_status = obj;
            lv_obj_set_pos(obj, 16, 148);
            lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "");
        }
        {
            // pw_keyboard
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.pw_keyboard = obj;
            lv_obj_set_pos(obj, 0, 353);
            lv_obj_set_size(obj, 320, 127);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_wifi_connect();
}

void tick_screen_wifi_connect() {
}

void create_screen_file_explorer() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.file_explorer = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // fe_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fe_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // fe_path
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fe_path = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "/");
        }
        {
            // fe_refresh_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fe_refresh_btn = obj;
            lv_obj_set_pos(obj, 290, 8);
            lv_obj_set_size(obj, 24, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "");
        }
        {
            // fe_list
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.fe_list = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 448);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj32 = obj;
                    lv_obj_set_pos(obj, 6, 4);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "  logs/");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj33 = obj;
                    lv_obj_set_pos(obj, 6, 22);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "  wifi_known.txt          248 B");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj34 = obj;
                    lv_obj_set_pos(obj, 6, 40);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "  config.cfg          1.0 KB  ");
                }
            }
        }
    }
    
    tick_screen_file_explorer();
}

void tick_screen_file_explorer() {
}

void create_screen_file_viewer() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.file_viewer = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // fv_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fv_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // fv_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fv_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "FILENAME.TXT");
        }
        {
            // fv_text
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.fv_text = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 448);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj35 = obj;
                    lv_obj_set_pos(obj, 6, 4);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Sample file content line 1");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj36 = obj;
                    lv_obj_set_pos(obj, 6, 22);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Sample file content line 2");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj37 = obj;
                    lv_obj_set_pos(obj, 6, 40);
                    lv_obj_set_size(obj, 308, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Sample file content line 3");
                }
            }
        }
    }
    
    tick_screen_file_viewer();
}

void tick_screen_file_viewer() {
}

void create_screen_i2c_spi_scanner() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.i2c_spi_scanner = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // scanner_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.scanner_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // scanner_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.scanner_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "I2C / SPI SCANNER");
        }
        {
            // mode_i2c_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.mode_i2c_btn = obj;
            lv_obj_set_pos(obj, 8, 32);
            lv_obj_set_size(obj, 50, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "I2C");
        }
        {
            // mode_spi_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.mode_spi_btn = obj;
            lv_obj_set_pos(obj, 62, 32);
            lv_obj_set_size(obj, 50, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SPI");
        }
        {
            // scanner_scan_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.scanner_scan_btn = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Scan");
        }
        {
            // found_count_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.found_count_label = obj;
            lv_obj_set_pos(obj, 8, 68);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "FOUND - 3 DEVICES");
        }
        {
            // device_list
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.device_list = obj;
            lv_obj_set_pos(obj, 0, 90);
            lv_obj_set_size(obj, LV_PCT(100), 130);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_scroll_dir(obj, LV_DIR_VER);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj38 = obj;
                    lv_obj_set_pos(obj, 8, 8);
                    lv_obj_set_size(obj, 304, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj39 = obj;
                            lv_obj_set_pos(obj, 12, 11);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "0x3C");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj40 = obj;
                            lv_obj_set_pos(obj, 80, 11);
                            lv_obj_set_size(obj, 160, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "SSD1306");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj41 = obj;
                            lv_obj_set_pos(obj, 232, 11);
                            lv_obj_set_size(obj, 64, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "OLED");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj42 = obj;
                    lv_obj_set_pos(obj, 8, 46);
                    lv_obj_set_size(obj, 304, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj43 = obj;
                            lv_obj_set_pos(obj, 12, 11);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "0x68");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj44 = obj;
                            lv_obj_set_pos(obj, 80, 11);
                            lv_obj_set_size(obj, 160, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "MPU6050");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj45 = obj;
                            lv_obj_set_pos(obj, 232, 11);
                            lv_obj_set_size(obj, 64, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "IMU");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj46 = obj;
                    lv_obj_set_pos(obj, 8, 84);
                    lv_obj_set_size(obj, 304, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj47 = obj;
                            lv_obj_set_pos(obj, 12, 11);
                            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "0x76");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj48 = obj;
                            lv_obj_set_pos(obj, 80, 11);
                            lv_obj_set_size(obj, 160, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "BME280");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj49 = obj;
                            lv_obj_set_pos(obj, 232, 11);
                            lv_obj_set_size(obj, 64, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Env");
                        }
                    }
                }
            }
        }
        {
            // addr_map_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.addr_map_title = obj;
            lv_obj_set_pos(obj, 8, 228);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ADDRESS MAP - 0X00 TO 0X7F");
        }
        {
            // addr_grid
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.addr_grid = obj;
            lv_obj_set_pos(obj, 8, 248);
            lv_obj_set_size(obj, 304, 222);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // addr_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.addr_hint = obj;
            lv_obj_set_pos(obj, 8, 343);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "tap address for datasheet - encoder scrolls list");
        }
    }
    
    tick_screen_i2c_spi_scanner();
}

void tick_screen_i2c_spi_scanner() {
}

void create_screen_serial_monitor() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.serial_monitor = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // serial_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.serial_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // serial_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.serial_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SERIAL MONITOR");
        }
        {
            // baud_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.baud_btn = obj;
            lv_obj_set_pos(obj, 8, 30);
            lv_obj_set_size(obj, 64, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "115200");
        }
        {
            // framing_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.framing_btn = obj;
            lv_obj_set_pos(obj, 78, 30);
            lv_obj_set_size(obj, 44, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "8N1");
        }
        {
            // rx_dot
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.rx_dot = obj;
            lv_obj_set_pos(obj, 134, 38);
            lv_obj_set_size(obj, 8, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // rx_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.rx_label = obj;
            lv_obj_set_pos(obj, 146, 34);
            lv_obj_set_size(obj, 24, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RX");
        }
        {
            // tx_dot
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tx_dot = obj;
            lv_obj_set_pos(obj, 178, 38);
            lv_obj_set_size(obj, 8, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tx_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tx_label = obj;
            lv_obj_set_pos(obj, 190, 34);
            lv_obj_set_size(obj, 24, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "TX");
        }
        {
            // lineend_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lineend_btn = obj;
            lv_obj_set_pos(obj, 248, 30);
            lv_obj_set_size(obj, 64, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "CR+LF");
        }
        {
            // term_output
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.term_output = obj;
            lv_obj_set_pos(obj, 8, 56);
            lv_obj_set_size(obj, 304, 342);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj50 = obj;
                    lv_obj_set_pos(obj, 8, 8);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[00:00:01] boot");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj51 = obj;
                    lv_obj_set_pos(obj, 8, 28);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[00:00:01] WiFi OK");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj52 = obj;
                    lv_obj_set_pos(obj, 8, 48);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[00:00:02] T=28.4 H=67");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj53 = obj;
                    lv_obj_set_pos(obj, 8, 68);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf59e0b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[00:00:05] WARN: Vbat low");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj54 = obj;
                    lv_obj_set_pos(obj, 8, 88);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf87171), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "[00:00:09] ERR: timeout");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj55 = obj;
                    lv_obj_set_pos(obj, 8, 108);
                    lv_obj_set_size(obj, 288, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "> _");
                }
            }
        }
        {
            // term_input
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.term_input = obj;
            lv_obj_set_pos(obj, 8, 406);
            lv_obj_set_size(obj, 228, 34);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "type a command...");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // send_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.send_btn = obj;
            lv_obj_set_pos(obj, 244, 412);
            lv_obj_set_size(obj, 68, 24);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Send");
        }
        {
            // macro_at_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.macro_at_btn = obj;
            lv_obj_set_pos(obj, 8, 450);
            lv_obj_set_size(obj, 40, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AT");
        }
        {
            // macro_rst_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.macro_rst_btn = obj;
            lv_obj_set_pos(obj, 54, 450);
            lv_obj_set_size(obj, 46, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RST");
        }
        {
            // macro_help_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.macro_help_btn = obj;
            lv_obj_set_pos(obj, 106, 450);
            lv_obj_set_size(obj, 48, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "help");
        }
        {
            // macro_clr_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.macro_clr_btn = obj;
            lv_obj_set_pos(obj, 160, 450);
            lv_obj_set_size(obj, 40, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "clr");
        }
        {
            // macro_add_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.macro_add_btn = obj;
            lv_obj_set_pos(obj, 206, 450);
            lv_obj_set_size(obj, 76, 22);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "+ macro");
        }
        {
            // serial_keyboard
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.serial_keyboard = obj;
            lv_obj_set_pos(obj, 0, 353);
            lv_obj_set_size(obj, 320, 127);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_serial_monitor();
}

void tick_screen_serial_monitor() {
}

void create_screen_wled_lights() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wled_lights = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // wled_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wled_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // wled_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wled_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "WLED LIGHTS");
        }
        {
            // wled_add_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wled_add_btn = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 60, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "+ Add");
        }
        {
            // all_lights_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.all_lights_card = obj;
            lv_obj_set_pos(obj, 8, 34);
            lv_obj_set_size(obj, 304, 56);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // all_lights_name
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.all_lights_name = obj;
                    lv_obj_set_pos(obj, 12, 9);
                    lv_obj_set_size(obj, 160, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "All Lights");
                }
                {
                    // all_lights_summary
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.all_lights_summary = obj;
                    lv_obj_set_pos(obj, 12, 31);
                    lv_obj_set_size(obj, 220, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "3 of 4 on - avg 62%");
                }
            }
        }
        {
            // devices_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.devices_label = obj;
            lv_obj_set_pos(obj, 8, 100);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DEVICES - 4");
        }
        {
            // wled_device_list
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wled_device_list = obj;
            lv_obj_set_pos(obj, 8, 122);
            lv_obj_set_size(obj, 304, 300);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj56 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 304, 64);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.obj57 = obj;
                            lv_obj_set_pos(obj, 12, 12);
                            lv_obj_set_size(obj, 16, 16);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa78bfa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj58 = obj;
                            lv_obj_set_pos(obj, 40, 10);
                            lv_obj_set_size(obj, 180, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Living Room Strip");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj59 = obj;
                            lv_obj_set_pos(obj, 40, 32);
                            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "192.168.1.42 - 144 LEDs");
                        }
                    }
                }
            }
        }
        {
            // wled_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wled_hint = obj;
            lv_obj_set_pos(obj, 8, 450);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "tap device for controls - + to discover");
        }
    }
    
    tick_screen_wled_lights();
}

void tick_screen_wled_lights() {
}

void create_screen_wled_detail() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wled_detail = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // detail_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.detail_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // detail_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.detail_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DEVICE");
        }
        {
            // detail_subtitle
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.detail_subtitle = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "host - 0 LEDs - online");
        }
        {
            // live_preview
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.live_preview = obj;
            lv_obj_set_pos(obj, 8, 52);
            lv_obj_set_size(obj, 304, 22);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa78bfa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // live_preview_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.live_preview_label = obj;
                    lv_obj_set_pos(obj, 0, 3);
                    lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "LIVE PREVIEW");
                }
            }
        }
        {
            // hue_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hue_label = obj;
            lv_obj_set_pos(obj, 8, 84);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "HUE");
        }
        {
            // hue_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.hue_slider = obj;
            lv_obj_set_pos(obj, 8, 102);
            lv_obj_set_size(obj, 304, 12);
            lv_slider_set_range(obj, 0, 359);
            lv_slider_set_value(obj, 180, LV_ANIM_OFF);
        }
        {
            // sat_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sat_label = obj;
            lv_obj_set_pos(obj, 8, 124);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SATURATION");
        }
        {
            // sat_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.sat_slider = obj;
            lv_obj_set_pos(obj, 8, 142);
            lv_obj_set_size(obj, 304, 12);
            lv_slider_set_range(obj, 0, 255);
            lv_slider_set_value(obj, 255, LV_ANIM_OFF);
        }
        {
            // bri_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.bri_label = obj;
            lv_obj_set_pos(obj, 8, 164);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BRIGHTNESS - 75%");
        }
        {
            // bri_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.bri_slider = obj;
            lv_obj_set_pos(obj, 8, 182);
            lv_obj_set_size(obj, 304, 12);
            lv_slider_set_range(obj, 0, 255);
            lv_slider_set_value(obj, 191, LV_ANIM_OFF);
        }
        {
            // quick_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.quick_label = obj;
            lv_obj_set_pos(obj, 8, 206);
            lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "QUICK COLORS");
        }
        {
            // quick_colors
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.quick_colors = obj;
            lv_obj_set_pos(obj, 8, 226);
            lv_obj_set_size(obj, 304, 40);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // solid_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.solid_card = obj;
            lv_obj_set_pos(obj, 8, 276);
            lv_obj_set_size(obj, 304, 73);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // solid_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.solid_label = obj;
                    lv_obj_set_pos(obj, 12, 8);
                    lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Solid color mode");
                }
                {
                    // solid_sub
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.solid_sub = obj;
                    lv_obj_set_pos(obj, 12, 30);
                    lv_obj_set_size(obj, 240, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "effects/presets - coming in a future update");
                }
            }
        }
        {
            // detail_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.detail_hint = obj;
            lv_obj_set_pos(obj, 8, 440);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "drag sliders to adjust - encoder -> brightness");
        }
    }
    
    tick_screen_wled_detail();
}

void tick_screen_wled_detail() {
}

void create_screen_wled_add() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wled_add = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // add_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.add_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // add_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.add_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ADD WLED DEVICE");
        }
        {
            // scan_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.scan_section = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 400);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // scan_status
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.scan_status = obj;
                    lv_obj_set_pos(obj, 0, 210);
                    lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Scanning network...");
                }
                {
                    // scan_sub
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.scan_sub = obj;
                    lv_obj_set_pos(obj, 0, 234);
                    lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Looking for WLED devices via mDNS");
                }
                {
                    // scan_timer
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.scan_timer = obj;
                    lv_obj_set_pos(obj, 0, 258);
                    lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "0.0s / 5s");
                }
                {
                    // scan_skip_btn
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.scan_skip_btn = obj;
                    lv_obj_set_pos(obj, 60, 300);
                    lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Skip - add manually instead");
                }
            }
        }
        {
            // results_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.results_section = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 400);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // results_banner
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.results_banner = obj;
                    lv_obj_set_pos(obj, 8, 8);
                    lv_obj_set_size(obj, 304, 28);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x10231a), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Found 2 devices on network");
                }
                {
                    // results_list
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.results_list = obj;
                    lv_obj_set_pos(obj, 8, 44);
                    lv_obj_set_size(obj, 304, 340);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.obj60 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 304, 56);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_obj_create(parent_obj);
                                    objects.obj61 = obj;
                                    lv_obj_set_pos(obj, 12, 16);
                                    lv_obj_set_size(obj, 16, 16);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xead9a0), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                                }
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.obj62 = obj;
                                    lv_obj_set_pos(obj, 40, 9);
                                    lv_obj_set_size(obj, 200, LV_SIZE_CONTENT);
                                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "wled-office");
                                }
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.obj63 = obj;
                                    lv_obj_set_pos(obj, 40, 31);
                                    lv_obj_set_size(obj, 240, LV_SIZE_CONTENT);
                                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "192.168.1.74 - 60 LEDs - FW 0.14.4");
                                }
                            }
                        }
                    }
                }
            }
        }
        {
            // empty_section
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.empty_section = obj;
            lv_obj_set_pos(obj, 0, 32);
            lv_obj_set_size(obj, 320, 400);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // empty_title
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.empty_title = obj;
                    lv_obj_set_pos(obj, 0, 120);
                    lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "No WLED devices found");
                }
                {
                    // empty_sub
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.empty_sub = obj;
                    lv_obj_set_pos(obj, 0, 144);
                    lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Scan completed - 0 devices responded");
                }
                {
                    // empty_tips
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.empty_tips = obj;
                    lv_obj_set_pos(obj, 16, 184);
                    lv_obj_set_size(obj, 288, 80);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "TRY THIS:\n- Check the WLED device is powered on\n- Confirm it's on the same WiFi\n- Some routers block mDNS across bands");
                }
            }
        }
        {
            // scan_again_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.scan_again_btn = obj;
            lv_obj_set_pos(obj, 8, 440);
            lv_obj_set_size(obj, 150, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Scan again");
        }
        {
            // add_manual_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.add_manual_btn = obj;
            lv_obj_set_pos(obj, 162, 440);
            lv_obj_set_size(obj, 150, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "+ Add manually");
        }
    }
    
    tick_screen_wled_add();
}

void tick_screen_wled_add() {
}

void create_screen_wled_add_manual() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wled_add_manual = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // manual_back_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.manual_back_btn = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, " Back");
        }
        {
            // manual_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.manual_title = obj;
            lv_obj_set_pos(obj, 0, 8);
            lv_obj_set_size(obj, 320, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ADD MANUALLY");
        }
        {
            // manual_input_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.manual_input_label = obj;
            lv_obj_set_pos(obj, 8, 32);
            lv_obj_set_size(obj, 304, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "IP ADDRESS OR HOSTNAME");
        }
        {
            // manual_input
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.manual_input = obj;
            lv_obj_set_pos(obj, 8, 52);
            lv_obj_set_size(obj, 304, 40);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "192.168.1.x");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f0f0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // manual_test_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.manual_test_btn = obj;
            lv_obj_set_pos(obj, 8, 100);
            lv_obj_set_size(obj, 304, 32);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x60a5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Test Connection");
        }
        {
            // manual_result
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.manual_result = obj;
            lv_obj_set_pos(obj, 8, 140);
            lv_obj_set_size(obj, 304, 56);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // manual_result_name
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.manual_result_name = obj;
                    lv_obj_set_pos(obj, 12, 9);
                    lv_obj_set_size(obj, 280, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "wled-garage");
                }
                {
                    // manual_result_info
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.manual_result_info = obj;
                    lv_obj_set_pos(obj, 12, 31);
                    lv_obj_set_size(obj, 280, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "120 LEDs - FW 0.14.4 - 84ms");
                }
            }
        }
        {
            // manual_add_btn
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.manual_add_btn = obj;
            lv_obj_set_pos(obj, 8, 206);
            lv_obj_set_size(obj, 304, 36);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x3ecf8e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111115), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x2a2a2e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Add Device");
        }
        {
            // manual_keyboard
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.manual_keyboard = obj;
            lv_obj_set_pos(obj, 0, 353);
            lv_obj_set_size(obj, 320, 127);
            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_wled_add_manual();
}

void tick_screen_wled_add_manual() {
}

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 0, 7);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text_static(obj, "{{screen_title}}");
        }
    }
}

void tick_user_widget_status_bar(int startWidgetIndex) {
    (void)startWidgetIndex;
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_settings,
    tick_screen_wifi_manager,
    tick_screen_logs,
    tick_screen_wifi_connect,
    tick_screen_file_explorer,
    tick_screen_file_viewer,
    tick_screen_i2c_spi_scanner,
    tick_screen_serial_monitor,
    tick_screen_wled_lights,
    tick_screen_wled_detail,
    tick_screen_wled_add,
    tick_screen_wled_add_manual,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 13) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_settings();
    create_screen_wifi_manager();
    create_screen_logs();
    create_screen_wifi_connect();
    create_screen_file_explorer();
    create_screen_file_viewer();
    create_screen_i2c_spi_scanner();
    create_screen_serial_monitor();
    create_screen_wled_lights();
    create_screen_wled_detail();
    create_screen_wled_add();
    create_screen_wled_add_manual();
}