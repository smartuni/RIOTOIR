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

static unsigned int isr_counter = 0;

static char timestamps[128];

static char stack[THREAD_STACKSIZE_MAIN];

static char ntp_stack[THREAD_STACKSIZE_MAIN];

gpio_t transistor_pin = GPIO_PIN(PA, 13);

static tsrb_t queue = TSRB_INIT(timestamps);

void *thread_handler(void *arg) {
    (void) arg;
    sock_udp_ep_t server = SOCK_IPV4_EP_ANY;
    uint64_t msg_payload[20];

    // send a timestamp for every use of 1Wh to cloud
    server.addr.ipv4[0] = 127;
    server.addr.ipv4[1] = 0;
    server.addr.ipv4[2] = 0;
    server.addr.ipv4[3] = 1;
    server.port = 7;

    sock_udp_t sock;
    if (sock_udp_create(&sock, NULL, &server, 0) < 0) {
        return NULL;
    }

    while (true) {
        xtimer_sleep(5);
        int count = tsrb_get(&queue,(char *) &msg_payload, sizeof(msg_payload));
        sock_udp_send(&sock, (void *) &msg_payload, sizeof(uint64_t) * count, NULL);
    }
    return NULL;
}

void *ntp_thread_handler(void *arg) {
    (void) arg;
    sock_udp_ep_t server = SOCK_IPV4_EP_ANY;

    // nutzt den HAW NPT Server 141.22.13.9
    server.addr.ipv4[0] = 141;
    server.addr.ipv4[1] = 22;
    server.addr.ipv4[2] = 13;
    server.addr.ipv4[3] = 9;
    server.port = 123;
    while (true) {
        sntp_sync(&server, 1000);
        xtimer_sleep(3600);
    }

}

void isr(void *arg) {
    (void) arg;

    uint64_t tmp = 0;
    if (isr_counter < 9) {
        ++isr_counter;
    } else {
        tmp = sntp_get_unix_usec();
        tsrb_add(&queue, (char *) &tmp , sizeof(tmp));
        isr_counter = 0;

    }
}

int main(void) {

    puts("This is the RIOTOIR: smart meter\n");

    thread_create(ntp_stack, sizeof(ntp_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, ntp_thread_handler,
                  NULL, "ntp_sync");

    thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_handler, NULL,
                  "time_printer");

    gpio_init_int(transistor_pin, GPIO_IN, GPIO_RISING, isr, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
