
#include "sendIR.h"
#include "crc8.h"
#include "xtimer.h"
#include "shell.h"
#include "thread.h"


static void sendPulse(uint32_t onTime_us){
	gpio_set(pin);								// IR off
	xtimer_usleep(PAUSE_TIME_US);
	gpio_clear(pin);							// IR on
	xtimer_usleep(onTime_us);
	gpio_set(pin);								// IR off
}

static void sendNibble(uint8_t nibble){
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

static void sendByte(uint8_t byte){
	sendNibble(byte >> 4);
	sendNibble(byte);
}

static void sendMsg(struct message *msg){

	uint8_t len = msg->length;
	uint8_t *msgPtr = (uint8_t *)msg;

	for(int i = 0; i < len; i++){
		sendByte( msgPtr[i] );
	}
}

void messageSend(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen){
	if(loadLen > MAX_PLD_LENGTH){
		perror("Message zu lang");
		//return EXIT_FAILURE;
	}

	uint8_t payloadAlloc[MAX_MSG_LENGTH];

	struct message *msg = (struct message *) payloadAlloc;
	msg->version = VERSION;
	msg->length = HEADER_LEN + loadLen;
	msg->reciver = reciver;
	msg->transmitter = transmitter;
	// getCRC8(const uint8_t *data, size_t data_len)
	msg->checksum = getCRC8( (uint8_t *) msg, (size_t)(HEADER_LEN - 1));

	memcpy(&(msg->payload), payload, loadLen);

	sendMsg(msg);
}

int main(void){

	puts("IR-send Messages");

	gpio_init(pin, GPIO_OUT);
    gpio_set(pin); 							// IR aus

	//sendByte(127);


    // Message senden
    // messageCreate(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen)
    char * str = "Hallo Marcel!";
    messageSend( 73, 68, ((uint8_t*)(str)), strlen(str) );

	/*
	// Sendet einen char
	while(true){
		char sign;
		printf("(%s)\n", "Geben sie eine Zahl ein (0 - 255):");
		scanf("%c", &sign);
		sendByte((uint8_t)sign);
	}
	*/

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

