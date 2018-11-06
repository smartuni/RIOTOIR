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

#define SIGNAL_DURATION_US 575
#define BIT_DURATION_US (2 * SIGNAL_DURATION_US)
#define BIT_DURATION_TOLLERANCE ((int) (SIGNAL_DURATION_US * 0.05))
#define BITS_PER_FRAME ((uint8_t) 10)
#define START_STOP_SIGNAL 1000

#define DECODER_MSG_QUEUE_SIZE 128
#define PRINTER_MSG_QUEUE_SIZE 2048

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

        printf("char received: %x (%c)\n", (char) msg.content.value, (char) msg.content.value);
    }
    return NULL;
}

void *thread_decoder(void *arg) {
    (void)arg;

    bool start_bit_recieved = false;
    // timestamp of the last pulse
    uint32_t last_pulse = 0;
    // count of received pulses
    uint8_t received_pulses = 0;
    // nibble we expect next (0 or 1)
    uint8_t nibble = 0;
    // receive buffer, flushed when a full byte is received
    uint8_t recv_buffer = 0;

    msg_init_queue(decoder_msg_queue, DECODER_MSG_QUEUE_SIZE);

    while(true) {
        msg_t msg;
        msg_receive(&msg);

        uint32_t pulse_timestamp = msg.content.value;
        bool pin_high = msg.content.value & 0x1;

        uint32_t diff = pulse_timestamp - last_pulse;

        if (!pin_high && !start_bit_recieved && diff >= START_STOP_SIGNAL) {
            start_bit_recieved = true;

        } else if (!pin_high && start_bit_recieved && diff >= START_STOP_SIGNAL) {
            --received_pulses;
            if (nibble == 0) {
                recv_buffer = received_pulses << 4;
                ++nibble;
            } else {
                recv_buffer |= received_pulses;
                nibble = 0;

                msg_t msg;
                msg.content.value = recv_buffer;
                if (msg_send(&msg, print_thread_pid) != 1) {
                    decode_error_cnt |= 2;
                }
            }
            received_pulses = 0;
            start_bit_recieved = 0;


        } else if (pin_high && start_bit_recieved) {
            ++received_pulses;

        } else {
            //TODO this is not an error case
            decode_error_cnt |= 1;
        }


        last_pulse = pulse_timestamp;
    }
}

void isr(void *arg) {
    (void)arg;
    const uint32_t timestamp = xtimer_now_usec();

    msg_t msg;
    // encode pin state in lsb
    msg.content.value = (timestamp & 0xFFFFFE) | gpio_read(transistor_pin);

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

    gpio_init_int(transistor_pin, GPIO_IN_PD, GPIO_BOTH, isr, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
