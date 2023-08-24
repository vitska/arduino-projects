#ifndef __WIFI_AP_SCAN_H__
#define __WIFI_AP_SCAN_H__

typedef struct {
  char ssid[21];
  char pass[21];
} wifi_ap_t;

extern wifi_ap_t* selected_wifi_ap;

#endif