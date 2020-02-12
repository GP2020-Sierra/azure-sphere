//

#include "lib_ccs811.h"
#include "sensors.h"
#include "onboard.h"
#include <time.h>

ccs811_t *p_ccs;
extern int i2cFd;

void ccs811Setup(void) {
    Log_Debug("Open CCS\n");
    p_ccs = ccs811_open(i2cFd, CCS811_I2C_ADDRESS_1, SK_SOCKET2_CS_GPIO);

    struct timespec sleepTime; //TODO: make nice sleepy function
    sleepTime.tv_sec = 1;
    sleepTime.tv_nsec = 0;

    ccs811_set_mode(p_ccs, CCS811_MODE_1S);

    nanosleep(&sleepTime, NULL);

    
    Log_Debug("CCS811 Calibrating...\n");
    ccs811_set_environmental_data(p_ccs, 22.0f, 30.0f);
    Log_Debug("CCS811 Calibrated\n");
}

CCS811Results_t readCCS811(void) {

    CCS811Results_t results;
    struct timespec sleepTime; //TODO: make nice sleepy function
    sleepTime.tv_sec = 1;
    sleepTime.tv_nsec = 0;
    nanosleep(&sleepTime, NULL);

    if (ccs811_get_results(p_ccs, &results.tvoc, &results.eco2, 0, 0)) {
        Log_Debug("CCS811 Sensor periodic: TVOC %d ppb, eCO2 %d ppm\n", results.tvoc, results.eco2);
    }
    else
    {
        Log_Debug("No results\n");
    }

    nanosleep(&sleepTime, NULL);

    Log_Debug("Close CCS\n");
    ccs811_close(p_ccs);

    return results;
}

SensorResults_t readSensors(void) {
    SensorResults_t results;

    results.ccs811results = readCCS811();
    results.onboardresults = readOnboardSensors();

    return results;
}

int setUpSensors(void) {
    ccs811Setup();
}