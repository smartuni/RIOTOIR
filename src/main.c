
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

uint32_t PULSE_WIDTH_TIME_US = 500;		//IR-on time (when sending '0'-Bit)
uint32_t BIT_TIME_US = 1000; 	 		//Transmittime for 1 Bit
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

int sendBit(uint8_t bit){
	int failure = 0;
	if(bit == 1){
		puts("bit 1\n");
		// Interrupt, wenn andere senden!
		gpio_set(pin);								// IR off
		xtimer_usleep(BIT_TIME_US);
		// prüfen ob andere gesendet haben
	} else if( (bit & 1)== 0) {
		puts("bit 0\n");
		// Interrupt, wenn andere senden!
		gpio_set(pin);								// IR off
		xtimer_usleep(PULSE_PAUSE_TIME_US);
		// prüfen ob andere gesendet haben
		gpio_clear(pin);							// IR on
		xtimer_usleep(PULSE_WIDTH_TIME_US);
		gpio_set(pin);								// IR off
		// Interrupt, wenn andere senden!
		xtimer_usleep(PULSE_PAUSE_TIME_US);
		// prüfen ob andere gesendet haben
	} else {
		error = 1;
	}

	if(failure == 0){
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}


int sendByte(uint8_t byte){
	// send StartBit
	if(sendBit(0) != EXIT_SUCCESS){
		error = 3;
		return EXIT_FAILURE;
	}

	for(int i = 7; i >= 0; --i ){
		if(sendBit((byte >> i) & 0x1) != EXIT_SUCCESS){
			error = 2;
			return EXIT_FAILURE;
		}
	}

	// send StopBit
	sendBit(1);
	return EXIT_SUCCESS;
}


int main(void){

	puts("IR-send 1ms on 1s off");

	gpio_init(pin, GPIO_OUT);
    gpio_set(pin); 							// Pin auf High

	sendByte(7);
	printf("error: %d\n", error);

/*
	uint32_t SLEEPTIME_CLEAR_US = 500; //min 500us entferung 15cm zur erkennung (test samr21-xpro)
	uint32_t SLEEPTIME_SET_US = 500; //min 500us entferung 15cm zur erkennung (test samr21-xpro)


	for(int i = 0; i < 1000; i++){
		LED0_TOGGLE;
		gpio_clear(pin);					// IR an
		xtimer_usleep(SLEEPTIME_CLEAR_US);
		gpio_set(pin);						// IR aus
		xtimer_usleep(SLEEPTIME_SET_US);
	}
*/
    return 0;
}

