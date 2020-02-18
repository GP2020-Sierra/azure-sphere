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
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/uart.h>


#include "sensors.h"
#include "uartMine.h"


// File descriptors - initialized to invalid value
extern int uartFd;

// State variables
static GPIO_Value_Type buttonState = GPIO_Value_High;

// Termination state
static volatile sig_atomic_t terminationRequired = false;

uint8_t* returnBuffer = 0;
int inputN = 0;
extern char* message;

void putTogether(uint8_t* buffer, int bytesRead) {
    if ( ((char *)buffer)[0] == "{"[0] || inputN > 0) {
        Log_Debug("current message: \"%s\",   buffer: \"%s\",   inputN %d, bytesRead %d", message, (char *)buffer, inputN, bytesRead);
        int n = sprintf(&message[inputN], "%s", (char *)buffer);
        Log_Debug(",   wrote %d\n", n);
        inputN += bytesRead;
        if (((char*)buffer)[bytesRead-2] == "}"[0]) {
            Log_Debug("\nCHARS %d MESSAGE %s\n", inputN, (char *)message);
            inputN = 0;
            JSON_Value *rootProperties = json_parse_string(message);
            if (rootProperties == NULL) {
                Log_Debug("WARNING: Cannot parse the string as JSON content.\n");
            }
        }
    }
}

void UartEventHandler(EventData *eventData) {
    const size_t receiveBufferSize = 256;
    uint8_t receiveBuffer[receiveBufferSize + 1]; // allow extra byte for string termination
    ssize_t bytesRead;

    // Read incoming UART data. It is expected behavior that messages may be received in multiple
    // partial chunks.
    bytesRead = read(uartFd, receiveBuffer, receiveBufferSize);
    if (bytesRead < 0) {
        Log_Debug("ERROR: Could not read UART: %s (%d).\n", strerror(errno), errno);
        terminationRequired = true;
        return;
    }

    if (bytesRead > 0) {
        // Null terminate the buffer to make it a valid string, and print it
        // Log_Debug("%c\n",((char *)receiveBuffer)[bytesRead-2]);
        // if ((char *)receiveBuffer[bytesRead-2] == "}"[0]) {Log_Debug("JSON end\n");}
        // if ((char *)receiveBuffer[0] == "{"[0]) { Log_Debug("JSON start\n");}
        receiveBuffer[bytesRead] = 0;
        //Log_Debug("UART received %d bytes: '%s'.\n", bytesRead, (char *)receiveBuffer);
        //Log_Debug("!!! %s\n", (char *)receiveBuffer);
        putTogether((char *)receiveBuffer, bytesRead);
        
    }
}

