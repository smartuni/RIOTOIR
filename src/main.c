#include <stdio.h>

#include "periph/gpio.h"
#include "shell.h"

#include "receive_ir.h"
#include "send_ir.h"

#define BUFFERSIZE 32


#define MSG_BUFFER_SIZE 255


gpio_t transistor_pin = GPIO_PIN(PA, 13);


int send_msg_handler(int argc, char **argv) {
    if (argc != 1) {
        puts("Illegal argument count");
        return 1;
    }

    messageSend(0, 0, (uint8_t *) argv[0], strlen(argv[0]));
}

static const shell_command_t commands[] = {
        {"sendmsg", "sends message via IR", send_msg_handler},
        { NULL, NULL, NULL }
};



int main(void)
{
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);

    setup_ir_recv(transistor_pin);
    setup_ir_send();


    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
