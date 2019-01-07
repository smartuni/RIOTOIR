/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "shell.h"
#include "Riotoir_display.h"

#include "receive_ir.h"
#include "send_ir.h"


#define BUFFERSIZE 32

#define MSG_BUFFER_SIZE 255

//IR pins
gpio_t receive_pin = GPIO_PIN(PA, 13);
gpio_t send_pin = GPIO_PIN(PA, 6);


int send_msg_handler( int argc, char** argv ) {
    if (argc != 2) {
        puts("Illegal argument count");
        return 1;
    }

    if (messageSend(0, 0, (uint8_t*) argv[1], strlen(argv[1])) != EXIT_SUCCESS) {
        puts("message to long");
        return -1;
    }

    return 0;
}


const shell_command_t shell_commands[] = {
        { "contrast", "set contrast",                contrast },
        { "temp",     "set temperature coefficient", temp },
        { "bias",     "set BIAS value",              bias },
        { "on",       "turn display on",             on },
        { "off",      "turn display off",            off },
        { "clear",    "clear memory",                clear },
        { "invert",   "invert display",              invert },
        { "riot",     "display RIOT logo",           riot },
        { "write",    "write string to display",     write },
        { "sendmsg",  "sends message via IR",        send_msg_handler },
        { NULL, NULL, NULL }
};

int main( void ) {
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);

    if (display_init() != 0) {
        return 1;
    }

    setup_ir_recv(receive_pin);
    setup_ir_send(send_pin);

    puts("All OK, running shell now");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
