//

#include "lib_ccs811.h"
#include "sensors.h"
#include "onboard.h"
#include <time.h>
#include "uartMine.h"

ccs811_t *p_ccs;
extern int i2cFd;
static long unsigned int resultCounter;


EspResults_t espresultsFromUart;
DhtResults_t dhtresultsFromUart;

/// <summary>
///     Sets up CCS811 ready to read from.
/// </summary>
void ccs811Setup(void) {
    Log_Debug("Open CCS\n");
    p_ccs = ccs811_open(i2cFd, CCS811_I2C_ADDRESS_1, SK_SOCKET2_CS_GPIO);

    ccs811_set_mode(p_ccs, CCS811_MODE_1S);
}

/// <summary>
///     Gets eCO2 and TVOC results from CCS811 sensor and returns them in sensor results struct.
/// </summary>
CCS811Results_t readCCS811(void) {

    CCS811Results_t results;

    if (ccs811_get_results(p_ccs, &results.tvoc, &results.eco2, 0, 0)) {
        Log_Debug("Read ccs");
    }
    else
    {
        Log_Debug("No results\n");
    }

    return results;
}

/// <summary>
///     Calls functions to read all sensors and puts the results from all into a big struct, which it returns.
/// </summary>
SensorResults_t readSensors(void) {
    SensorResults_t results;
    results.timestamp = time(NULL);

    results.ccs811results = readCCS811();
    results.onboardresults = readOnboardSensors();

    Log_Debug("Onboard Sensor: Temperature 1 %f, Temperature 2 %f\nCCS811 Sensor periodic: TVOC %d ppb, eCO2 %d ppm\n", results.onboardresults.lps22hhTemperature_degC, results.onboardresults.lsm6dsoTemperature_degC, results.ccs811results.tvoc, results.ccs811results.eco2);

    results.espresults = espresultsFromUart;
    results.dhtresults = dhtresultsFromUart;

    Log_Debug("ESP ts %d dev %d DHT ts %d temp %f\n", results.espresults.timestamp, results.espresults.devices, results.dhtresults.timestamp, results.dhtresults.dhtTemperature_degC);

    results.counter = resultCounter; // used so we know how many times the sensors have been read since device last restarted, as first results often less accurate
    resultCounter++;

    // if (resultCounter % 10 == 0) {
    //         //calibrate the co2 sensor to make predictions better
    //         ccs811_set_environmental_data(p_ccs, dhtresultsFromUart.dhtTemperature_degC, dhtresultsFromUart.humidity);
    // }

    return results;
}

// TODO: move init peripherals and handlers here
int setUpSensors(void) {
    ccs811Setup();
    return 0;
}

int closeSensors(void) {
    // TODO: need to close ccs
    // TODO: work out if this is still needed
    return 0;
}