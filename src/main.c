
#include "shell.h"
#include "thread.h"
#include "gpio.h"
#include "xtimer.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>



gpio_t pin = GPIO_PIN(PA, 6);			//GPIO Pin declaration

uint32_t PULSE_WIDTH_TIME_NS = 217;		//IR-on time (when sending '0'-Bit)
uint32_t BIT_TIME_NS = 1157; 	 		//Transmittime for 1 Bit 
uint32_t PULSE_PAUSE_TIME_NS = 470;     //Pause, before and behind PULSE_WIDTH_TIME

uint8_t HEADER_LEN = 6 * sizeof(uint8_t);
uint8_t MAX_MSG_LENGTH = 0xff - ( HEADER_LEN );
uint8_t PRAEAMBLE = 1 + 4 + 16 + 64; // 10101010
uint8_t VERSION = 0;

struct message{
	uint8_t praeamble;
	uint8_t version;
	uint8_t length;
	uint8_t	reciver;
	uint8_t transmitter;
	uint8_t checksum;
	uint8_t * payload;
}__attribute__ ((packed));

struct message* messageCreate(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen){
	if(loadLen > MAX_MSG_LENGTH){
		perror("Message zu lang");
		return EXIT_FAILURE;
	}
	struct message msg;
	msg.praeamble = PRAEAMBLE;
	msg.version = VERSION;
	msg.length = HEADER_LEN + loadLen;
	msg.reciver = reciver;
	msg.transmitter = transmitter; 
	msg.checksum = getChecksum();    //TODO  
	msg.payload = payload;

	return msg*;
}

int sendBit(uint8_t bit){
	int failure = 0;
	if(bit == 1){
		// Interrupt, wenn andere senden!
		gpio_set(pin);								// IR off
		xtimer_nanosleep(BIT_TIME_NS);
		// prüfen ob andere gesendet haben
	} else if( (bit && 1) == 0) {
		// Interrupt, wenn andere senden!
		gpio_set(pin);								// IR off
		xtimer_nanosleep(PULSE_PAUSE_TIME_NS);
		// prüfen ob andere gesendet haben
		gpio_clear(pin);							// IR on
		xtimer_nanosleep(PULSE_WIDTH_TIME_NS);		
		gpio_set(pin);								// IR off
		// Interrupt, wenn andere senden!
		xtimer_nanosleep(PULSE_PAUSE_TIME_NS);
		// prüfen ob andere gesendet haben
	}
	
	if(failure == 0){
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}


int sendByte(uint8_t byte){
	if(sendBit(0)){								// send StartBit
		return EXIT_FAILURE;
	}
	for(int i = 0; i < 8; i++ ){
		if(sendBit(byte >> i) != EXIT_SUCCESS){
			return EXIT_FAILURE;
		}
	}
	sendBit(1);									// send StopBit
	return EXIT_SUCCESS;
}


int main(void){
	
	puts("IR-send 1ms on 1s off");
	
	uint32_t SLEEPTIME_CLEAR_MS = 1;
	uint32_t SLEEPTIME_SET_MS = 1000;
    
	gpio_init(pin, GPIO_OUT_PU);	
    gpio_set(pin); 							// Pin auf High
	
	while(1){
		LED0_TOGGLE
		gpio_clear(pin);					// IR an 
		xtimer_usleep(SLEEPTIME_CLEAR_MS);
		gpio_set(pin);						// IR aus
		xtimer_usleep(SLEEPTIME_SET_MS);	
	}
	
    return 0;
}
