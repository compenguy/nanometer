#ifndef __WIFI_H__
#define __WIFI_H__

#include <stdint.h>

bool check_wifi_init();
void setup_wifi();
void set_wifi_lowPower();
void set_wifi_normalPower();
bool check_wifi_failed();
bool check_wifi_disconnected();
bool check_wifi_connected();
uint8_t get_wifi_disconnectReason();
uint32_t get_wifi_unixEpochTime();

#endif  // __WIFI_H__
