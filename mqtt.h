#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>
#include <limits.h>

void setup_mqtt();
bool check_mqtt_connected();
void mqtt_publish_temp(int32_t new_temp_dC);

#endif  // __MQTT_H__