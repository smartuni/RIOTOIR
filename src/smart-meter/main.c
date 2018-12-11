#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "xtimer.h"
#include "thread.h"
#include "periph/gpio.h"
#include "net/sock.h"
#include "net/sock/udp.h"
#include "net/sntp.h"
#include "tsrb.h"
#include "fmt.h"

static uint32_t isr_counter = 0;

static char stack[THREAD_STACKSIZE_MAIN];

gpio_t transistor_pin = GPIO_PIN(PA, 18);

void* thread_handler( void* arg ) {
    (void) arg;
    unsigned int Wh = 0;
    uint32_t old_counter = 0;
    uint32_t new_counter = 0;
    uint64_t old_timestamp = sntp_get_unix_usec();
    uint64_t new_timestamp = 0;

    while (true) {
        xtimer_sleep(1);

        new_counter = isr_counter;
        new_timestamp = sntp_get_unix_usec();

        char print[21];
        print[20] = '\0';
        fmt_u64_dec((char*) &print, ( new_timestamp - old_timestamp ));
        old_timestamp = new_timestamp;
        if (new_counter > old_counter) {
            Wh = new_counter - old_counter;
            old_counter = new_counter;
        } else if (new_counter < old_counter) {
            Wh = UINT32_MAX - old_counter + new_counter;
            old_counter = new_counter;
        } else {
            Wh = 0;
        }

        // Testing
        printf("used %d 100mWh in %s us\n", Wh, print);

    }

    return EXIT_SUCCESS;
}

void isr( void* arg ) {
    (void) arg;
    ++isr_counter;
}

int main( void ) {

    puts("This is the RIOTOIR: smart meter\n");

    thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_handler, NULL,
                  "x * 100mWh consumed printer");

    if (gpio_init_int(transistor_pin, GPIO_IN_PD, GPIO_RISING, isr, NULL) < 0) {
        puts("Interrupt init error\n");
    }

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
