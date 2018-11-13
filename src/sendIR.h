#ifndef SENDIR_H_
#define SENDIR_H_


#include "shell.h"
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>


uint32_t PAUSE_TIME_US = 500;			//
uint32_t PULSE_WIDTH_TIME_US = 500;		//IR-on time (when sending '0'-Bit)
uint32_t START_END_PULSE_US = 1000; 	//Transmittime for start- and endpulse
uint32_t PULSE_PAUSE_TIME_US = 250;     //Pause, before and behind PULSE_WIDTH_TIME

gpio_t pin = GPIO_PIN(PA, 6);			//GPIO Pin declaration

#define HEADER_LEN (5 * sizeof(uint8_t))
static const uint8_t MAX_PLD_LENGTH = 0xff - ( HEADER_LEN );
static const uint8_t MAX_MSG_LENGTH = 0xff;
static const uint8_t VERSION = 0;

struct message{
	uint8_t version;
	uint8_t	reciver;
	uint8_t transmitter;
	uint8_t length;
	uint8_t checksum;
	uint8_t payload[];
}__attribute__ ((packed));

/**
* Builds a Message with Header and Payload and send it 
*/
void messageSend(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen);



#endif // SENDIR_H_