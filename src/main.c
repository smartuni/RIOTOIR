#include <stdio.h>
#include <string.h>
#include <math.h>

#include "led.h"
#include "msg.h"
#include "periph/gpio.h"
#include "shell.h"
#include "thread.h"
#include "xtimer.h"

#define BUFFERSIZE 32

#define SIGNAL_DURATION_US 500
#define BIT_DURATION_US 2 * SIGNAL_DURATION_US
#define BIT_DURATION_TOLLERANCE ((int) (SIGNAL_DURATION_US * 0.05))
#define BITS_PER_FRAME 10

#define DECODER_MSG_QUEUE_SIZE 128
#define PRINTER_MSG_QUEUE_SIZE 16

static char stack[THREAD_STACKSIZE_MAIN];
static char stack_decoder[THREAD_STACKSIZE_MAIN];
static msg_t decoder_msg_queue[DECODER_MSG_QUEUE_SIZE];
static msg_t printer_msg_queue[PRINTER_MSG_QUEUE_SIZE];


static unsigned int isr_counter = 0;
static unsigned int recv_error_cnt = 0;
static unsigned int decode_error_cnt = 0;

kernel_pid_t decoder_thread_pid;
kernel_pid_t print_thread_pid;

gpio_t transistor_pin = GPIO_PIN(PA, 13);

void *thread_handler(void *arg) {
    (void)arg;

    msg_init_queue(printer_msg_queue, PRINTER_MSG_QUEUE_SIZE);


    while(true) {
        msg_t msg;
        msg_receive(&msg);

        printf("char received: %x (%c) ", (char) msg.content.value, (char) msg.content.value);

        printf("%ld: %d\n", xtimer_now_usec(), isr_counter);
        printf("%d\n", gpio_read(transistor_pin));
    }
    return NULL;
}

void *thread_decoder(void *arg) {
    (void)arg;

    static bool start_byte_recieved = false;
    static uint32_t last_pulse = 0;
    uint8_t received_bits = 0;
    uint8_t recv_buffer = 0;

    msg_init_queue(decoder_msg_queue, DECODER_MSG_QUEUE_SIZE);

    while(true) {
        xtimer_usleep(BIT_DURATION_US + BIT_DURATION_TOLLERANCE);

        // no pulse received and no frame started
        if (!msg_avail() && !start_byte_recieved) {
            continue;

        // pulse indicates start of frame
        } else if (msg_avail() && !start_byte_recieved) {
            msg_t msg;
            msg_receive(&msg);

            last_pulse = msg.content.value;
            start_byte_recieved = true;
            ++received_bits;

        // no pulse received and at the end of a frame
        } else if (!msg_avail() && start_byte_recieved && received_bits >= 9) {
            received_bits = 0;
            start_byte_recieved = false;

            msg_t msg;
            msg.content.value = (uint32_t) recv_buffer;
            msg_send(&msg, print_thread_pid);

            recv_buffer = 0;

        // no pulse received and in the middle of a frame
        // indicating value 1 bit
        } else if (!msg_avail() && start_byte_recieved) {
            ++received_bits;
            recv_buffer |= 0b1 << ( 7 - (received_bits  - 1));

        // pulse received and in the middle of a frame
        } else if (msg_avail() && start_byte_recieved) {
            msg_t msg;
            msg_receive(&msg);
            // TODO do something with this timestamp
            uint32_t pulse = msg.content.value;
            ++received_bits;
            last_pulse = pulse;

        // Should never be reached!!!
        } else {
            ++decode_error_cnt;
        }
    }
}

void isr(void *arg) {
    (void)arg;
    const uint32_t timestamp = xtimer_now_usec();

    msg_t msg;
    msg.content.value = timestamp;

    if (msg_send(&msg, decoder_thread_pid) != 1) {
        ++recv_error_cnt;
    }

    ++isr_counter;
}

int main(void)
{
    puts("This is the RIOTOIR project\n");
    printf("This application runs on %s\n", RIOT_BOARD);

    print_thread_pid = thread_create(
            stack, sizeof(stack),
            THREAD_PRIORITY_MAIN - 1,
            THREAD_CREATE_STACKTEST,
            thread_handler, NULL,
            "time_printer");

    decoder_thread_pid = thread_create(
            stack_decoder, sizeof(stack_decoder),
            THREAD_PRIORITY_MAIN - 2,
            THREAD_CREATE_STACKTEST,
            thread_decoder,
            NULL,
            "decoder");

    gpio_init_int(transistor_pin, GPIO_IN, GPIO_RISING, isr, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
