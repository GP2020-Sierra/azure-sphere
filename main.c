

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>


#include <hw/sample_hardware.h>

int main(void)
{
    Log_Debug("Starting CMake Board and Sensor Test application...\n");

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
        GPIO_SetValue(fd2, GPIO_Value_Low);
        GPIO_SetValue(fd, GPIO_Value_High);
        nanosleep(&sleepTime, NULL);
        GPIO_SetValue(fd, GPIO_Value_Low);
        GPIO_SetValue(fd2, GPIO_Value_High);
        nanosleep(&sleepTime, NULL);
    }
}