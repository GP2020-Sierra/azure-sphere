/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This minimal Azure Sphere app repeatedly toggles GPIO 8, which is the red channel of RGB
// LED 1 on the MT3620 RDB. Use this app to test that device and SDK installation succeeded
// that you can build, deploy, and debug a CMake app with Visual Studio.
//
// It uses the API for the following Azure Sphere application libraries:
// - gpio (digital input for button)
// - log (messages shown in Visual Studio's Device Output window during debugging)

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "applibs_versions.h"
#include <applibs/log.h>
#include <applibs/gpio.h>
#include <applibs/uart.h>

// By default, this sample's CMake build targets hardware that follows the MT3620
// Reference Development Board (RDB) specification, such as the MT3620 Dev Kit from
// Seeed Studios.
//
// To target different hardware, you'll need to update the CMake build. The necessary
// steps to do this vary depending on if you are building in Visual Studio, in Visual
// Studio Code or via the command line.
//
// See https://github.com/Azure/azure-sphere-samples/tree/master/Hardware for more details.
//
// This #include imports the sample_hardware abstraction from that hardware definition.
#include <hw/sample_hardware.h>

#define PROJECT_ISU2_I2C MT3620_RDB_HEADER4_ISU2_I2C

#include "lib_ccs811.h"
#include "onboard.h"
#include "sensors.h"
#include "messages.h"
#include "uartMine.h"

// Set to change frequency of readings and sending
#define TIME_BETWEEN_READINGS 10
// time in seconds
#define READINGS_BEFORE_SEND 6

// Support functions.
static void TerminationHandler(int signalNumber);
static int InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);

// File descriptors - initialized to invalid value
int i2cFd = -1;
int uartFd = -1;
int epollFd = -1;

// Termination state
volatile sig_atomic_t terminationRequired = false;

// event handler data structures. Only the event handler field needs to be populated.
static EventData uartEventData = {.eventHandler = &UartEventHandler};

/// <summary>
///     Signal handler for termination requests. This handler must be 
///     async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, it is not guaranteed to be async-signal-safe.
    terminationRequired = true;
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals and 
///     set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static int InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    epollFd = CreateEpollFd();
    if (epollFd < 0) {
        return -1;
    }

    // Init I2C
    i2cFd = I2CMaster_Open(PROJECT_ISU2_I2C);
    if (i2cFd < 0) {
        Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno,
            strerror(errno));
        return -1;
    }

    int result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetBusSpeed: errno=%d (%s)\n", errno,
            strerror(errno));
        return -1;
    }

    result = I2CMaster_SetTimeout(i2cFd, 100);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n", errno,
            strerror(errno));
        return -1;
    }

    result = I2CMaster_SetDefaultTargetAddress(i2cFd, 0x3C);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetDefaultTargetAddress: errno=%d (%s)\n",
            errno, strerror(errno));
        return -1;
    }

    initOnboardI2c();

    // // Create a UART_Config object, open the UART and set up UART event handler
    UART_Config uartConfig;
    UART_InitConfig(&uartConfig);
    uartConfig.baudRate = 115200;
    uartConfig.flowControl = UART_FlowControl_None;
    uartFd = UART_Open(SAMPLE_UART, &uartConfig);
    if (uartFd < 0) {
        Log_Debug("ERROR: Could not open UART: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    if (RegisterEventHandlerToEpoll(epollFd, uartFd, &uartEventData, EPOLLIN) != 0) {
        return -1;
    }
    UartSetup();

    return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void) {
    close(i2cFd);
}


void sendResults(SensorResults_t* results, int resultsLen) {
    const unsigned int messageSize = 500;
    char* csv = (char *)malloc(messageSize);
    // timestamp, tempLPS, tempLSM, tempDHT, pressure, humidity, eco2, tvoc
    int n = 0;
    n = sprintf(csv, "{\"data\": \"timestamp,count,tempLPS,tempLSM,tempDHT,pressure,humidity,eco2,tvoc\\n");
    for (int i = 0; i < resultsLen; i++) {
        SensorResults_t result = results[i];
        n += sprintf(&csv[n], "%u,%u,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\\n", result.timestamp, result.counter, result.onboardresults.lps22hhTemperature_degC, result.onboardresults.lsm6dsoTemperature_degC, 0.0, 0.0, 0.0, result.ccs811results.eco2, result.ccs811results.tvoc);
    }
    n += sprintf(&csv[n], "\"}");
    if (n<1) {
        Log_Debug("CSV string write failed.");
    } else {
        Log_Debug("CSV string write succeeded");
    }
    SendTelemetryCSV(csv);
    free(csv);
}

/// <summary>
///     Application main entry point
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("\n*** Starting ***\n");

    if (InitPeripheralsAndHandlers() != 0)
    {
        terminationRequired = true;
    }

    setUpMessages(argc, argv);

    if (!terminationRequired) {
        setUpSensors();
        SensorResults_t result;
        //char tempBuffer[20];
        SensorResults_t* resultsToSend = (SensorResults_t *)malloc(sizeof(SensorResults_t) * READINGS_BEFORE_SEND);
        int n = 0;
        while(1) {
            if((time(NULL) - result.timestamp) >= TIME_BETWEEN_READINGS) {
                Log_Debug("Time for next reading\n");
                result = readSensors();
                //len = snprintf(tempBuffer, 20, "%3.2f", results.onboardresults.lps22hhTemperature_degC);
                resultsToSend[n] = result;
                n += 1;
                if (n==READINGS_BEFORE_SEND) {
                    Log_Debug("Time to send\n");
                    sendResults(resultsToSend, READINGS_BEFORE_SEND);
                    n = 0;
                }
                iotConnect();
            } else {
                struct timespec sleepTime;
                sleepTime.tv_sec = 1;
                sleepTime.tv_nsec = 1000000; // 10ms
                nanosleep(&sleepTime, NULL);
                WaitForEventAndCallHandler(epollFd);
            }
        }      
        free(resultsToSend);  
    }

    Log_Debug("*** Terminating ***\n");
    ClosePeripheralsAndHandlers();
    return 0;
}
