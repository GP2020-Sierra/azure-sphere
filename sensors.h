#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "onboard.h"

typedef struct CCS811Results {
    uint16_t tvoc;
    uint16_t eco2;
} CCS811Results_t;

typedef struct EspResults {
    time_t timestamp;
    int devices;
    int basestations;
} EspResults_t;

typedef struct DhtResults {
    time_t timestamp;
    int humidity;
    int dhtTemperature_degC;
} DhtResults_t;

typedef struct SensorResults {
    time_t timestamp;
    long unsigned int counter;
    CCS811Results_t ccs811results;
    OnboardResults_t onboardresults;
    EspResults_t espresults;
    DhtResults_t dhtresults;
} SensorResults_t;


SensorResults_t readSensors(void);

int setUpSensors(void);