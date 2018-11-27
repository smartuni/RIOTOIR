#ifndef SENDIR_H_
#define SENDIR_H_


#include "periph/gpio.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/**
* Builds a Message with Header and Payload and send it
*/
void messageSend(uint8_t transmitter, uint8_t reciver, uint8_t * payload, uint8_t loadLen);

void setup_ir_send(void);



#endif // SENDIR_H_
