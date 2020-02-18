#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include "epoll_timerfd_utilities.h"
#include "parson.h"
#include <applibs/log.h>
#include <applibs/uart.h>

#include "sensors.h"
#include "uartMine.h"


// File descriptors - initialized to invalid value
extern int uartFd;

// Termination state
extern volatile sig_atomic_t terminationRequired;

#define BUFFER_SIZE 1024
static size_t bufferUsed = 0;
static unsigned char* mainBuffer;
static unsigned char* copyBuffer;

void UartSetup() {
    mainBuffer = malloc(BUFFER_SIZE);
    copyBuffer = malloc(BUFFER_SIZE);
}

static void processLine(void) {
    // Log_Debug("[UART]: %s", mainBuffer);
    
    JSON_Value *rootProperties = json_parse_string(mainBuffer);
    if (rootProperties == NULL) {
        // if we can't parse it, it's probably just a debug message we can safely ignore
        return;
    }

    JSON_Object *rootObject = json_value_get_object(rootProperties);
    const char* sensor = json_object_dotget_string(rootObject, "sensor");
    Log_Debug("Received new data from %s over UART\n", sensor); // probably want to comment this out later - lots of spam

    // TODO depending on the sensor type, update a global variable storing the last read sensor values.
    // then take/pass a copy of this in sensors.c
}

static int findLine(void) {
    for (size_t i = 0; i <= bufferUsed; i++) {
        if (mainBuffer[i] == '\n') {
            mainBuffer[i] = '\0';
            processLine();

            memcpy(copyBuffer, &mainBuffer[i+1], BUFFER_SIZE - bufferUsed - 1);
            memcpy(mainBuffer, copyBuffer, BUFFER_SIZE - bufferUsed - 1);
            bufferUsed -= i + 1;
        } else if (mainBuffer[i] == '\'') {
            // esp code sends "JSON" but uses ' instead of "
            // so convert them here
            mainBuffer[i] = '"';
        }
    }

    return 0;
}

void UartEventHandler(EventData *eventData) {
    // Read incoming UART data. It is expected behavior that messages may be received in multiple
    // partial chunks.
    ssize_t bytesRead = read(uartFd, &mainBuffer[bufferUsed], BUFFER_SIZE - bufferUsed - 1);
    if (bytesRead < 0) {
        Log_Debug("ERROR: Could not read UART: %s (%d).\n", strerror(errno), errno);
        terminationRequired = true;
        return;
    }

    if (bytesRead > 0) {
        // Null terminate the buffer to make it a valid string, and print it
        bufferUsed += (size_t) bytesRead;
        mainBuffer[bufferUsed + 1] = 0;
        while (findLine()) {};
    }
}

