#pragma once

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

int setUpMessages(int argc, char *argv[]);

void SendTelemetry(const unsigned char *key, const unsigned char *value);

void TerminationHandler(int signalNumber)