#include "mqtt.h"

// requires SECRET_MQTT_BROKER_URI, SECRET_MQTT_BROKER_PORT, SECRET_MQTT_USER, SECRET_MQTT_PASS defined in nanometer_secrets.h
#include "nanometer_secrets.h"

#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char mqtt_broker[] = SECRET_MQTT_BROKER_URI;
int mqtt_port = SECRET_MQTT_BROKER_PORT;
const char mqtt_user[] = SECRET_MQTT_USER;
const char mqtt_pass[] = SECRET_MQTT_PASS;
const char topic_temp[] = "office_sensor/temperature_dC";
const char topic_hum[] = "office_sensor/humidity_pct";
int32_t last_temp_dC = INT_MIN;

void setup_mqtt() {
  mqttClient.setUsernamePassword(mqtt_user, mqtt_pass);
  mqttClient.setId("nanometer");
  Serial.println("[mqtt   ] Initializing connection");
  if (!mqttClient.connect(mqtt_broker, mqtt_port)) {
    Serial.print("[mqtt   ] connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  } else {
    Serial.println("[mqtt   ] connection completed");
  }
}

bool check_mqtt_connected() {
  return mqttClient.connected();
}

void mqtt_publish_temp(int32_t new_temp_dC) {
  mqttClient.poll();
  if (new_temp_dC != INT_MIN && new_temp_dC != last_temp_dC) {
    mqttClient.beginMessage(topic_temp);
    mqttClient.print(new_temp_dC);
    mqttClient.endMessage();
    Serial.print("[mqtt   ] Published new value ");
    Serial.print(new_temp_dC);
    Serial.print(" to topic ");
    Serial.println(topic_temp);
    last_temp_dC = new_temp_dC;
  }
}