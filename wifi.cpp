#include "wifi.h"

// requires SECRET_SSID and SECRET_PSK defined in nanometer_secrets.h
#include "nanometer_secrets.h"

#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>

static const char ssid[] = SECRET_SSID;
static const char psk[] = SECRET_PSK;

static void log_wifi();
static bool check_wifi_present();
static bool check_wifi_firmware();

bool check_wifi_init() {
  if (!check_wifi_present()) {
    Serial.println("Communication with WiFi module failed!");
    return false;
  }

  String version = WiFi.firmwareVersion();
  Serial.print("WiFi firmware version: ");
  Serial.println(version);
  Serial.print("Latest firmware version: ");
  Serial.println(WIFI_FIRMWARE_LATEST_VERSION);

  return true;
}

void setup_wifi() {
  int status = WL_IDLE_STATUS;  // wifi radio status

  while (!check_wifi_connected()) {
    Serial.print("Initiating connection with ssid ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA); // switch off AP
    status = WiFi.begin(ssid, psk);
    Serial.print("WiFi status: ");
    Serial.println(status);
    // Poll every second to see if we've connected, up to 10 seconds
    for (int i = 0; i < 10 && !check_wifi_connected(); i++) {
      Serial.println("Waiting for wifi to come up");
      delay(1000);
    }
  }
  set_wifi_lowPower();
  log_wifi();
}

void set_wifi_lowPower() {
  WiFi.lowPowerMode();
}

void set_wifi_normalPower() {
  WiFi.noLowPowerMode();
}

bool check_wifi_present() {
  int status = WiFi.status();
  Serial.print("WiFi status: ");
  Serial.println(status);
  return status != WL_NO_MODULE;
}

bool check_wifi_failed() {
  return WiFi.status() == WL_CONNECT_FAILED;
}

bool check_wifi_disconnected() {
  return WiFi.status() == WL_DISCONNECTED;
}

bool check_wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}

uint8_t get_wifi_disconnectReason() {
  return WiFi.reasonCode();
}

uint32_t get_wifi_unixEpochTime() {
  return WiFi.getTime();
}

void log_wifi() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}
