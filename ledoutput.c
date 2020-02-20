#include <applibs/gpio.h>
#include <hw/sample_hardware.h>
#include "ledoutput.h"

extern int bluefd;
extern int redfd;
extern int greenfd;

void setLedFds(void) {
    bluefd = GPIO_OpenAsOutput(MT3620_RDB_LED1_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
    redfd = GPIO_OpenAsOutput(MT3620_RDB_LED1_RED, GPIO_OutputMode_PushPull, GPIO_Value_High);
    greenfd = GPIO_OpenAsOutput(MT3620_RDB_LED1_GREEN, GPIO_OutputMode_PushPull, GPIO_Value_High);
}

void ledHappy(void){

    GPIO_SetValue(redfd, GPIO_Value_High);
    GPIO_SetValue(bluefd, GPIO_Value_High);
    GPIO_SetValue(greenfd, GPIO_Value_Low);
    //green
}

void ledAngry(void){
    GPIO_SetValue(greenfd, GPIO_Value_High);
    GPIO_SetValue(bluefd, GPIO_Value_High);
    GPIO_SetValue(redfd, GPIO_Value_Low);
    //red
}

void ledUnsure(void) {
    GPIO_SetValue(redfd, GPIO_Value_High);
    GPIO_SetValue(greenfd, GPIO_Value_High);
    GPIO_SetValue(bluefd, GPIO_Value_Low);
    // yellow = red+green
    // or just green = red+green :(
}