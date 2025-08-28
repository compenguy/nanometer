#include "wifi.h"
#include "lsm6dsox_temp.h"

#include <stdint.h>

void setup() {
  while (!check_wifi_init())
    ;
  setup_wifi();
  set_wifi_lowPower();
}

void loop() {
  char textbuf[64] = { 0 };
  int32_t temp_dC = INT_MIN;
  int32_t last_temp_dC = INT_MIN;

  temp_dC = get_temperature_dC();
  if (temp_dC != INT_MIN && temp_dC != last_temp_dC) {
    snprintf(textbuf, sizeof(textbuf), "IMU temperature: %ddC", temp_dC);
    Serial.println(textbuf);
  }

  if (!check_wifi_connected()) {
    snprintf(textbuf, sizeof(textbuf), "Lost WiFi connection (reason %d)", get_wifi_disconnectReason());
    Serial.println(textbuf);
  }

  // Wait one minute before the next reading
  // TODO: instead of doing the connected check at the beginning, do it here, and attempt reconnection
  // during the delay
  delay(60 * 1000);
}