#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "led.h"
#include "xtimer.h"
#include "thread.h"
#include "periph/gpio.h"
#include "tsrb.h"

static unsigned int isr_counter = 0;

static char stack[THREAD_STACKSIZE_MAIN];

gpio_t transistor_pin = GPIO_PIN(PA, 13);

void *thread_handler(void *arg)
{
    (void)arg;
	unsigned int kWh = 0;
    while(true) {
        xtimer_sleep(10);
        unsigned int counter = isr_counter;

		if( counter > 0 && counter % 10000 == 0) {
			kWh++;
		}
        printf("%ld: %d : %d kW/h\n", xtimer_now_usec(), counter, kWh);
        LED0_TOGGLE;
    }
    return NULL;
}

void isr(void *arg) {
    (void)arg;

    ++isr_counter;
}

int main(void)
{
    puts("This is the RIOTOIR: smart meter\n");

    thread_create(stack, sizeof(stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  thread_handler, NULL,
                  "time_printer");

    gpio_init_int(transistor_pin, GPIO_IN, GPIO_RISING, isr, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
