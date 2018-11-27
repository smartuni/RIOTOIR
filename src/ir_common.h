#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <stdint.h>


#define PAUSE_TIME_US 500           //
#define PULSE_WIDTH_TIME_US 500     //IR-on time (when sen
#define START_END_PULSE_US 1000     //Transmittime for start- and endpulse
#define PULSE_PAUSE_TIME_US 250     //Pause, before and behind PULSE_WIDTH_TIME


#define IR_PROTOCOL_VERSION 0


struct ir_header {
    uint8_t version;
    uint8_t receiver;
    uint8_t sender;
    uint8_t length;
    uint8_t checksum;
} __attribute__((packed));


struct ir_package {
    struct ir_header header;
    uint8_t msg[];
} __attribute__((packed));


#define MAX_PAYLOAD_LENGTH (UINT8_MAX - sizeof(struct ir_header))

#endif
