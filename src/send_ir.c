#include <alloca.h>
#include <stdlib.h>

#include "send_ir.h"
#include "crc8.h"
#include "xtimer.h"
#include "shell.h"
#include "thread.h"


#include "ir_common.h"


static gpio_t pin;

static void sendPulse( uint32_t onTime_us ) {
    gpio_set(pin);                              // IR off
    xtimer_usleep(PAUSE_TIME_US);
    gpio_clear(pin);                            // IR on
    xtimer_usleep(onTime_us);
    gpio_set(pin);                              // IR off
}

static void sendNibble( uint8_t nibble ) {
    // send StartBit
    sendPulse(START_END_PULSE_US);

    // set MSB-Nibble to zero
    nibble = nibble & 0xf;

    for (int i = 0; i < nibble; i++) {
        sendPulse(PULSE_WIDTH_TIME_US);
    }

    // send StopBit
    sendPulse(START_END_PULSE_US);
}

static void sendByte( uint8_t byte ) {
    sendNibble(byte >> 4);
    sendNibble(byte);
}

static void sendMsg( struct ir_package* msg ) {

    uint8_t len = msg->header.length;
    uint8_t* msgPtr = (uint8_t*) msg;

    for (int i = 0; i < len; i++) {
        sendByte(msgPtr[i]);
    }
}

int messageSend( uint8_t transmitter, uint8_t reciver, uint8_t* payload, uint8_t loadLen ) {
    if (loadLen > MAX_PAYLOAD_LENGTH) {
        perror("Message zu lang");
        return EXIT_FAILURE;
    }


    struct ir_package* msg = alloca(sizeof(struct ir_header) + loadLen);
    msg->header.version = IR_PROTOCOL_VERSION;
    msg->header.length = sizeof(struct ir_header) + loadLen;
    msg->header.receiver = reciver;
    msg->header.sender = transmitter;
    // getCRC8(const uint8_t *data, size_t data_len)
    msg->header.checksum = getCRC8((uint8_t*) msg, sizeof(struct ir_header) - 1);

    memcpy(&( msg->msg ), payload, loadLen);

    sendMsg(msg);

    return EXIT_SUCCESS;
}

void setup_ir_send( gpio_t send_pin ) {
    gpio_init(send_pin, GPIO_OUT);
    // IR off
    gpio_set(send_pin);

    pin = send_pin;
}

