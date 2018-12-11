#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "zaehlerSimulator.h"
#include "periph/gpio.h"

int main(void)
{
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);
    
    gpio_t pin = GPIO_PIN(PA, 5);
    
    init(pin);
    
    puts("begin.....\n");
    simConstFast(100);
    puts("....finish sending!\n");
    
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    
    return 0;
}
