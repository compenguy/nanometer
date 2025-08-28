#ifndef __LSM6DSOX_TEMP_H__
#define __LSM6DSOX_TEMP_H__

#include <limits.h>
#include <stdint.h>

// get temperature in tenths of degrees C
// if no temperature was available, returns INT_MIN
int32_t get_temperature_dC();

#endif  // __LSM6DSOX_TEMP_H__
