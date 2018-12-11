#include "zaehlerSimulator.h"


#include <stdio.h>
#include <string.h>
#include <xtimer.h>


#define SIM_PULSE_DURATION_US 250
#define SIM_MIN_PULSE_TO_PULSE_US (9000 - SIM_PULSE_DURATION_US)
#define SIM_PULSE_PER_KWH 10000
#define SIM_PULSE_PER_WH (SIM_PULSE_PER_KWH/1000)

gpio_t simulatorPin;


void init(gpio_t pin){
    simulatorPin = pin;
    gpio_init(simulatorPin, GPIO_OUT);
    gpio_set(simulatorPin);
}

void blink(void){
    gpio_clear(simulatorPin);                          // IR on
    xtimer_usleep(SIM_PULSE_DURATION_US);
    gpio_set(simulatorPin);                            // IR off
}

void simConstFast(int wh){
    for(int i = 0; i < wh * SIM_PULSE_PER_WH; i++){
        blink();
        xtimer_usleep(SIM_MIN_PULSE_TO_PULSE_US);
    }
}

void simConstSlow(int wh){
    for(int i = 0; i < wh * SIM_PULSE_PER_WH; i++){
        blink();
        xtimer_usleep(SIM_MIN_PULSE_TO_PULSE_US*10);
    }
}

void simDecIncFunc(int wh){
    int i = 0;
    int blinks = wh * SIM_PULSE_PER_WH;
    int halfBlinks = blinks/2;
    int curSleepTime = SIM_MIN_PULSE_TO_PULSE_US; 
    while( i < halfBlinks){
        blink();
        xtimer_usleep(curSleepTime);
        curSleepTime = curSleepTime * 1.2;
    }
    
    while( i < blinks){
        blink();
        xtimer_usleep(curSleepTime);
        curSleepTime = curSleepTime / 1.2;
    }
}


