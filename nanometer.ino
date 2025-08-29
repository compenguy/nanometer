

#include "wifi.h"
#include "mqtt.h"
#include "lsm6dsox_temp.h"

#include <stdint.h>
#include <limits.h>

// set interval for wakeups, sending updates
const uint32_t interval = 60 * 1000;
uint32_t previousMillis = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  while (!setup_temp())
    ;
  while (!check_wifi_init())
    ;
}

void loop() {
  char textbuf[64] = { 0 };
  int32_t temp_dC = INT_MIN;
  int32_t last_temp_dC = INT_MIN;

  while (!check_mqtt_connected()) {
    Serial.println("mqtt not connected");
    while (!check_wifi_connected()) {
      snprintf(textbuf, sizeof(textbuf), "Lost WiFi connection (reason %d)", get_wifi_disconnectReason());
      Serial.println(textbuf);
      Serial.println("Reconnecting...");
      setup_wifi();
      delay(1000);
    }
    setup_mqtt();
    delay(1000);
  }

  // We want each iteration to take _interval_ amount of millis
  // start counting from now
  uint32_t previousMillis = millis();
  temp_dC = get_temperature_dC();
  Serial.print("Temperature in deciCelsius: ");
  Serial.println(temp_dC);
  if (temp_dC != INT_MIN && temp_dC != last_temp_dC) {
    snprintf(textbuf, sizeof(textbuf), "IMU temperature: %d.%dC", temp_dC / 10, temp_dC % 10);
    Serial.println(textbuf);
    mqtt_publish_temp(temp_dC);
  }

  // Wait until one minute has expired before the next reading
  uint32_t currentMillis = millis();
  uint32_t elapsedMillis = currentMillis - previousMillis;
  if (elapsedMillis < interval) {
    Serial.print("Sleeping for millis: ");
    Serial.println(interval - elapsedMillis);
    delay(interval - elapsedMillis);
  }
  previousMillis = currentMillis;
}