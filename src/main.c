
#include "shell.h"
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define EXIT_SUCCESS 1
#define EXIT_FAILURE 0


gpio_t pin = GPIO_PIN(PA, 6);			//GPIO Pin declaration

uint32_t PAUSE_TIME_US = 500;			//
uint32_t PULSE_WIDTH_TIME_US = 500;		//IR-on time (when sending '0'-Bit)
uint32_t START_END_PULSE_US = 1000; 	 		//Transmittime for 1 Bit
uint32_t PULSE_PAUSE_TIME_US = 250;     //Pause, before and behind PULSE_WIDTH_TIME

#define HEADER_LEN (6 * sizeof(uint8_t))
static const uint8_t MAX_MSG_LENGTH = 0xff - ( HEADER_LEN );
static const uint8_t PRAEAMBLE = 1 + 4 + 16 + 64; // 10101010
static const uint8_t VERSION = 0;

uint8_t error = 0;

struct message{
	uint8_t praeamble;
	uint8_t version;
	uint8_t length;
	uint8_t	reciver;
	uint8_t transmitter;
	uint8_t checksum;
	uint8_t * payload;
}__attribute__ ((packed));

//TODO struct als pointer übergeben
struct message messageCreate(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen){
	if(loadLen > MAX_MSG_LENGTH){
		perror("Message zu lang");
		//return EXIT_FAILURE;
	}
	struct message msg;
	msg.praeamble = PRAEAMBLE;
	msg.version = VERSION;
	msg.length = HEADER_LEN + loadLen;
	msg.reciver = reciver;
	msg.transmitter = transmitter;
	msg.checksum = 1; //getChecksum();    //TODO
	msg.payload = payload;

	return msg;
}

void sendPulse(uint32_t onTime_us){
	gpio_set(pin);								// IR off
	xtimer_usleep(PAUSE_TIME_US);
	gpio_clear(pin);							// IR on
	xtimer_usleep(onTime_us);
	gpio_set(pin);								// IR off
}

void sendNibble(uint8_t nibble){
	// send StartBit
	sendPulse(START_END_PULSE_US);

	// set MSB-Nibble to zero
	nibble = nibble & 0xf; 

	for(int i = 0; i < nibble; i++){
		sendPulse(PULSE_WIDTH_TIME_US);
	}

	// send StopBit
	sendPulse(START_END_PULSE_US);	
}

void sendByte(uint8_t byte){
	sendNibble(byte >> 4);
	sendNibble(byte);
}


int main(void){

	puts("IR-send 1ms on 1s off");

	gpio_init(pin, GPIO_OUT);
    gpio_set(pin); 							// IR aus

	sendByte(127);
	printf("error: %d\n", error);


	//uint32_t SLEEPTIME_CLEAR_US = 500; //min 500us entferung 15cm zur erkennung (test samr21-xpro)
	//uint32_t SLEEPTIME_SET_US = 500; //min 500us entferung 15cm zur erkennung (test samr21-xpro)

	/*
	for(int i = 0; i < 1000; i++){
		LED0_TOGGLE;
		gpio_clear(pin);							// IR an
		xtimer_usleep(SLEEPTIME_CLEAR_US);
		gpio_set(pin);						// IR aus
		xtimer_usleep(SLEEPTIME_SET_US);
	}
	*/

    return 0;
}

