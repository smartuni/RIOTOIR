#include <stdio.h>

#include "periph/gpio.h"
#include "shell.h"

#include "ir.h"
#define BUFFERSIZE 32


#define MSG_BUFFER_SIZE 255


gpio_t transistor_pin = GPIO_PIN(PA, 13);


int main(void)
{
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);

    setup_ir_recv(transistor_pin);


    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
