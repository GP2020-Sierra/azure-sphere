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

#include <applibs/log.h>
#include <applibs/gpio.h>

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

// Support functions.
static void TerminationHandler(int signalNumber);
static int InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);

// File descriptors - initialized to invalid value
static int i2cFd = -1;

// Termination state
static volatile sig_atomic_t terminationRequired = false;

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

    return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void) {
    close(i2cFd);
}

/// <summary>
///     CCS811 demo application
/// </summary>
void ccs811Main(void) {

    ccs811_t *p_ccs;
    Log_Debug("Open CCS\n");
    p_ccs = ccs811_open(i2cFd, CCS811_I2C_ADDRESS_1, SK_SOCKET2_CS_GPIO);

    struct timespec sleepTime;
    sleepTime.tv_sec = 1;
    sleepTime.tv_nsec = 0;

    uint16_t tvoc;
    uint16_t eco2;
    ccs811_set_mode(p_ccs, CCS811_MODE_1S);

    nanosleep(&sleepTime, NULL);

    //ccs811_get_results(p_ccs, &tvoc, &eco2, 0, 0); // Ignore first reading as is borkened

    Log_Debug("CCS811 Calibrating...\n");
    ccs811_set_environmental_data(p_ccs, 22.0f, 30.0f);
    Log_Debug("CCS811 Calibrated\n");

    nanosleep(&sleepTime, NULL);

    for (int meas = 0; meas < 1000; meas++)
    {
        if (ccs811_get_results(p_ccs, &tvoc, &eco2, 0, 0)) {
            Log_Debug("CCS811 Sensor periodic: TVOC %d ppb, eCO2 %d ppm\n",
                tvoc, eco2);
        }
        else
        {
            Log_Debug("No results\n");
        }

        nanosleep(&sleepTime, NULL);
    }

    Log_Debug("Close CCS\n");
    ccs811_close(p_ccs);
}

/// <summary>
///     Application main entry point
/// </summary>
int main(void)
{
    Log_Debug("\n*** Starting ***\n");

    if (InitPeripheralsAndHandlers() != 0)
    {
        terminationRequired = true;
    }

    if (!terminationRequired) {
        ccs811Main();
    }

    Log_Debug("*** Terminating ***\n");
    ClosePeripheralsAndHandlers();
    return 0;
}

/*
int main(void)
{
    Log_Debug("Starting CMake Hello World application...\n");

    int fd = GPIO_OpenAsOutput(MT3620_RDB_LED1_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
    int fd2 = GPIO_OpenAsOutput(MT3620_RDB_LED1_RED, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (fd < 0 || fd2 < 0) {
        Log_Debug(
            "Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n",
            strerror(errno), errno);
        return -1;
    }
    GPIO_SetValue(fd, GPIO_Value_Low);
    GPIO_SetValue(fd2, GPIO_Value_Low);

    const struct timespec sleepTime = {1, 0};
    while (true) {
        GPIO_SetValue(fd2, GPIO_Value_High);
        GPIO_SetValue(fd, GPIO_Value_High);
        nanosleep(&sleepTime, NULL);
        GPIO_SetValue(fd, GPIO_Value_High);
        GPIO_SetValue(fd2, GPIO_Value_High);
        nanosleep(&sleepTime, NULL);
    }
}
*/