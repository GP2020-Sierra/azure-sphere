#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "onboard.h"

typedef struct CCS811Results {
    uint16_t tvoc;
    uint16_t eco2;
} CCS811Results_t;

typedef struct SensorResults {
    CCS811Results_t ccs811results;
    OnboardResults_t onboardresults;
} SensorResults_t;

SensorResults_t readSensors(void);

int setUpSensors(void);