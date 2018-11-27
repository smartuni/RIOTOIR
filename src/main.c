#include <stdio.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "shell.h"

#include "receive_ir.h"
#include "send_ir.h"

#define BUFFERSIZE 32


#define MSG_BUFFER_SIZE 255


gpio_t receive_pin = GPIO_PIN(PA, 13);
gpio_t send_pin = GPIO_PIN(PA, 6);


int send_msg_handler(int argc, char **argv) {
    if (argc != 2) {
        puts("Illegal argument count");
        return 1;
    }

    if (messageSend(0, 0, (uint8_t *) argv[1], strlen(argv[1])) != EXIT_SUCCESS) {
        puts("message to long");
        return -1;
    }

    return 0;
}

static const shell_command_t commands[] = {
        {"sendmsg", "sends message via IR", send_msg_handler},
        { NULL, NULL, NULL }
};



int main(void)
{
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);

    setup_ir_recv(receive_pin);
    setup_ir_send(send_pin);


    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
