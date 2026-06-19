#include "apps/wifi_manager/wifi_manager.h"

// Make LDF compile+link the arduino-esp32 3.x "Network" library that WiFi
// depends on (WiFi declares no `depends`, so it isn't pulled in otherwise).
// The matching `-I .../Network/src` in platformio.ini lets the WiFi library's
// own sources find these headers.
#include <Network.h>
#include <WiFi.h>
#include <stdlib.h>
#include <string.h>

#include "ui/eez_export/screens.h"
#include "ui/styles.h"
#include "core/screen_manager.h"
#include "apps/wifi_connect/wifi_connect.h"

// Polls for async-scan completion; non-null only while a scan is in flight.
static lv_timer_t *s_poll_timer = nullptr;
// Refreshes the connection/IP line at the top; lives as long as the screen.
static lv_timer_t *s_ip_timer = nullptr;

static const char *auth_str(wifi_auth_mode_t m)
{
  switch (m) {
    case WIFI_AUTH_OPEN:            return "Open";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/3";
    default:                        return "?";
  }
}

// Stronger signal -> more active bars (1..4) and a "healthier" color.
static int rssi_bars(int rssi)
{
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -75) return 2;
  return 1;
}

static lv_color_t rssi_color(int rssi)
{
  if (rssi >= -55) return styles::accent_green();
  if (rssi >= -65) return styles::accent_amber();
  if (rssi >= -75) return styles::accent_red();
  return styles::text_muted();
}

// Each card owns a strdup'd copy of its SSID in user_data (the scan results are
// freed right after the cards are built), released when the card is deleted.
static void card_delete_cb(lv_event_t *e)
{
  lv_obj_t *card = (lv_obj_t *)lv_event_get_target(e);
  free(lv_obj_get_user_data(card));
}

// Tapping a card opens the password/connect screen for that network.
static void card_click_cb(lv_event_t *e)
{
  lv_obj_t *card = (lv_obj_t *)lv_event_get_target(e);
  const char *ssid = (const char *)lv_obj_get_user_data(card);
  screen_manager::push(wifi_connect_create(ssid ? ssid : ""));
}

// Builds one network card matching the EEZ demo card visual, using the shared
// palette. Children are absolutely positioned within the 300x78 card; the card
// itself is laid out by the parent list's flex column.
static void make_network_card(lv_obj_t *parent, const char *ssid, int rssi,
                              int channel, wifi_auth_mode_t enc, const char *bssid)
{
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, 300, 78);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_style_bg_color(card, styles::bg_card(), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(card, 8, 0);
  lv_obj_set_style_border_color(card, styles::border(), 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_pad_all(card, 0, 0);

  // The whole card is tappable -> opens the connect screen for this SSID.
  lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_user_data(card, strdup((ssid && ssid[0]) ? ssid : ""));
  lv_obj_add_event_cb(card, card_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(card, card_delete_cb, LV_EVENT_DELETE, NULL);

  const int bars = rssi_bars(rssi);
  const lv_color_t col = rssi_color(rssi);

  // Signal-strength bars: 4 rects (heights 6/12/18/24), bottom-aligned.
  lv_obj_t *bc = lv_obj_create(card);
  lv_obj_set_size(bc, 26, 24);
  lv_obj_set_pos(bc, 12, 27);
  lv_obj_remove_flag(bc, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_remove_flag(bc, LV_OBJ_FLAG_CLICKABLE); // let taps fall through to card
  lv_obj_set_style_bg_opa(bc, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(bc, 0, 0);
  lv_obj_set_style_pad_all(bc, 0, 0);

  const int h[4] = {6, 12, 18, 24};
  for (int i = 0; i < 4; i++) {
    lv_obj_t *bar = lv_obj_create(bc);
    lv_obj_set_size(bar, 5, h[i]);
    lv_obj_set_pos(bar, i * 7, 24 - h[i]);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(bar, i < bars ? col : styles::border(), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(bar, 1, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
  }

  lv_obj_t *name = lv_label_create(card);
  lv_obj_set_pos(name, 50, 8);
  lv_obj_set_width(name, 170);
  lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(name, styles::text_primary(), 0);
  lv_label_set_text(name, (ssid && ssid[0]) ? ssid : "(hidden)");

  lv_obj_t *detail = lv_label_create(card);
  lv_obj_set_pos(detail, 50, 32);
  lv_obj_set_width(detail, 170);
  lv_label_set_long_mode(detail, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(detail, styles::text_secondary(), 0);
  lv_label_set_text_fmt(detail, "ch %d | %s | 2.4GHz", channel, auth_str(enc));

  lv_obj_t *mac = lv_label_create(card);
  lv_obj_set_pos(mac, 50, 52);
  lv_obj_set_width(mac, 180);
  lv_label_set_long_mode(mac, LV_LABEL_LONG_DOT);
  lv_obj_set_style_text_color(mac, lv_color_hex(0x666666), 0);
  lv_label_set_text(mac, bssid ? bssid : "");

  lv_obj_t *r = lv_label_create(card);
  lv_obj_set_pos(r, 228, 8);
  lv_obj_set_width(r, 60);
  lv_obj_set_style_text_align(r, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_text_color(r, col, 0);
  lv_label_set_text_fmt(r, "%d", rssi);
}

// Replaces the list contents with a single muted centered placeholder line.
static void show_placeholder(const char *msg)
{
  lv_obj_clean(objects.network_list);
  lv_obj_t *l = lv_label_create(objects.network_list);
  lv_obj_set_style_text_color(l, styles::text_muted(), 0);
  lv_label_set_text(l, msg);
}

static void poll_scan_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) {
    return; // keep polling
  }

  // Scan resolved (success or failure): stop polling.
  lv_timer_delete(s_poll_timer);
  s_poll_timer = nullptr;

  if (n < 0) { // WIFI_SCAN_FAILED or unexpected
    lv_label_set_text(objects.network_count, "SCAN FAILED");
    show_placeholder("Scan failed - tap Scan to retry");
    WiFi.scanDelete();
    return;
  }

  lv_obj_clean(objects.network_list);
  for (int i = 0; i < n; i++) {
    make_network_card(objects.network_list, WiFi.SSID(i).c_str(),
                      WiFi.RSSI(i), WiFi.channel(i), WiFi.encryptionType(i),
                      WiFi.BSSIDstr(i).c_str());
  }
  if (n == 0) {
    show_placeholder("No networks found");
  }
  lv_label_set_text_fmt(objects.network_count, "%d NETWORKS", n);
  WiFi.scanDelete();
}

static void scan_event_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_poll_timer) {
    return; // a scan is already running
  }
  lv_label_set_text(objects.network_count, "SCANNING...");
  show_placeholder("Scanning for networks...");
  WiFi.scanDelete();
  WiFi.scanNetworks(/*async=*/true, /*show_hidden=*/false);
  s_poll_timer = lv_timer_create(poll_scan_cb, 400, NULL);
}

// Keeps the top line in sync with the radio: the connected IP (green) or a
// muted "Not connected". Runs while the screen exists, so it also reflects a
// join made on the connect screen once we return here.
static void ip_refresh_cb(lv_timer_t *t)
{
  LV_UNUSED(t);
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text_fmt(objects.ip_label, LV_SYMBOL_WIFI "  %s",
                          WiFi.localIP().toString().c_str());
    lv_obj_set_style_text_color(objects.ip_label, styles::accent_green(), 0);
  } else {
    lv_label_set_text(objects.ip_label, "Not connected");
    lv_obj_set_style_text_color(objects.ip_label, styles::text_muted(), 0);
  }
}

// Tear down both timers if the screen is deleted, so neither can fire against
// freed widgets after a Back navigation.
static void screen_delete_cb(lv_event_t *e)
{
  LV_UNUSED(e);
  if (s_poll_timer) {
    lv_timer_delete(s_poll_timer);
    s_poll_timer = nullptr;
  }
  if (s_ip_timer) {
    lv_timer_delete(s_ip_timer);
    s_ip_timer = nullptr;
  }
  WiFi.scanDelete();
}

lv_obj_t *wifi_manager_create()
{
  create_screen_wifi_manager();

  lv_obj_add_style(objects.wifi_manager, &styles::style_screen, 0);

  // wifi_back_btn is an EEZ label; make it tappable (the log_back_btn pattern).
  lv_obj_add_flag(objects.wifi_back_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(
      objects.wifi_back_btn,
      [](lv_event_t *e) { LV_UNUSED(e); screen_manager::pop(); },
      LV_EVENT_CLICKED, NULL);

  // Bring up the radio in station mode for scanning. Scanning works fine on
  // an already-connected interface, so don't disconnect here - this screen is
  // recreated every time it's reopened (screen_manager deletes it on pop) and
  // an unconditional disconnect() was dropping any active connection.
  WiFi.mode(WIFI_STA);

  // Turn the EEZ list container into a scrolling flex column and drop the
  // design-time demo cards; populated dynamically on Scan.
  lv_obj_set_flex_flow(objects.network_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(objects.network_list, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(objects.network_list, 8, 0);
  lv_obj_set_style_pad_top(objects.network_list, 0, 0);
  lv_obj_set_style_pad_bottom(objects.network_list, 8, 0);
  show_placeholder("Tap Scan to search for networks");
  lv_label_set_text(objects.network_count, "TAP SCAN");

  // The "Scan" element is an EEZ label; make it tappable.
  lv_obj_add_flag(objects.scan_btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(objects.scan_btn, scan_event_cb, LV_EVENT_CLICKED, NULL);

  // Top connection/IP line, refreshed once a second (also catches a join made
  // on the connect screen after we return here).
  ip_refresh_cb(nullptr);
  s_ip_timer = lv_timer_create(ip_refresh_cb, 1000, NULL);

  lv_obj_add_event_cb(objects.wifi_manager, screen_delete_cb, LV_EVENT_DELETE,
                      NULL);

  return objects.wifi_manager;
}
