#include "lsm6dsox_temp.h"

#include <Arduino.h>
#include <Arduino_LSM6DSOX.h>

int32_t get_temperature_dC() {
  if (IMU.temperatureAvailable()) {
    float temp_C;
    IMU.readTemperatureFloat(temp_C);
    int32_t temp_dC = (int)(temp_C * 10);
    return (int32_t)(temp_C * 10);
  } else {
    return INT_MIN;
  }
}
