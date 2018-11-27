#include "send_ir.h"
#include "crc8.h"
#include "xtimer.h"
#include "shell.h"
#include "thread.h"


uint32_t PAUSE_TIME_US = 500;           //
uint32_t PULSE_WIDTH_TIME_US = 500;     //IR-on time (when sending '0'-Bit)
uint32_t START_END_PULSE_US = 1000;     //Transmittime for start- and endpulse
uint32_t PULSE_PAUSE_TIME_US = 250;     //Pause, before and behind PULSE_WIDTH_TIME

gpio_t pin = GPIO_PIN(PA, 6);           //GPIO Pin declaration

#define HEADER_LEN (5 * sizeof(uint8_t))
static const uint8_t MAX_PLD_LENGTH = 0xff - ( HEADER_LEN );
static const uint8_t MAX_MSG_LENGTH = 0xff;
static const uint8_t VERSION = 0;

struct message{
    uint8_t version;
    uint8_t reciver;
    uint8_t transmitter;
    uint8_t length;
    uint8_t checksum;
    uint8_t payload[];
}__attribute__ ((packed));


static void sendPulse(uint32_t onTime_us){
    gpio_set(pin);                              // IR off
    xtimer_usleep(PAUSE_TIME_US);
    gpio_clear(pin);                            // IR on
    xtimer_usleep(onTime_us);
    gpio_set(pin);                              // IR off
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

void setup_ir_send(void){


    gpio_init(pin, GPIO_OUT);
    // IR off
    gpio_set(pin);
}

