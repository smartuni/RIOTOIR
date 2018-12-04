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

static unsigned int isr_counter = 0;

static char timestamps[128];

static char stack[THREAD_STACKSIZE_MAIN];

//static char ntp_stack[THREAD_STACKSIZE_MAIN];

gpio_t transistor_pin = GPIO_PIN(PA, 18);

static tsrb_t queue = TSRB_INIT(timestamps);

void *thread_handler(void *arg) {
    (void) arg;
    //sock_udp_ep_t server = SOCK_IPV4_EP_ANY;
    uint64_t msg_payload[40];


    // send a timestamp for every use of 1Wh to cloud
    /*ipv4_addr_t server_addr;
    ipv4_addr_from_str(&server_addr, "127.0.0.1");
    server.addr.ipv4_u32 = server_addr.u32.u32;
    server.port = 7;

    sock_udp_t sock;
    if (sock_udp_create(&sock, NULL, NULL, 0) < 0) {
        perror("no udp socket created!");
        return NULL;
    }
     */

    while (true) {
        xtimer_sleep(5);
        int count = tsrb_get(&queue,(char *) &msg_payload, sizeof(msg_payload));
        if(count > 0) {
            /*if (sock_udp_send(&sock, (void *) &msg_payload, sizeof(uint64_t) * count, &server) < 0) {
                perror("no timestamps send per udp");
            }*/
            // Testing
            for (int i = 0; i < count / 8; ++i) {
                char print[22];
                print[21] = '\0';
                fmt_u64_dec((char*)&print, msg_payload[i]);
                printf("used 1Wh at %s\n", print);
            }
        } else {
            printf("nix in der queue");
        }
    }
    return NULL;
}

/*void *ntp_thread_handler(void *arg) {
    (void) arg;
    sock_udp_ep_t server = SOCK_IPV4_EP_ANY;

    // nutzt den HAW NPT Server 141.22.13.9
    ipv4_addr_t sntp_server_addr;
    ipv4_addr_from_str(&sntp_server_addr, "141.22.13.9");
    server.addr.ipv4_u32 = sntp_server_addr.u32.u32;
    server.port = 123;
    while (true) {
        int send_err = sntp_sync(&server, 10);
        if(send_err < 0) {
            perror("unix time sync error");
        }
        xtimer_sleep(1);
        printf("err:%d time: %llu\n", send_err, sntp_get_unix_usec());
    }

}*/

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

    //thread_create(ntp_stack, sizeof(ntp_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, ntp_thread_handler,
    //              NULL, "ntp_sync");

    thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_handler, NULL,
                  "Wh consume printer");

    if(gpio_init_int(transistor_pin, GPIO_IN_PD, GPIO_RISING, isr, NULL) < 0){
        puts("Interrupt init Fehler");
    }

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
