#pragma once

#include <stdbool.h>

#define LSM6DSO_ID         0x6C   // register value
#define LSM6DSO_ADDRESS	   0x6A	  // I2C Address

typedef struct OnboardResults {
    double acceleration_mg[3];
    double angular_rate_dps[3];
    float lsm6dsoTemperature_degC;
    float pressure_hPa;
    float lps22hhTemperature_degC;
} OnboardResults_t;

int initOnboardI2c(void);
OnboardResults_t readOnboardSensors(void);