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
#define BITS_PER_FRAME ((uint8_t) 10)

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
        xtimer_sleep(1);
        /*msg_t msg;
        msg_receive(&msg);

        printf("char received: %x (%c)\n", (char) msg.content.value, (char) msg.content.value);*/

        printf("%ld: %d recv_error_cnt: %d\n", xtimer_now_usec(), isr_counter, recv_error_cnt);
        //printf("%d\n", gpio_read(transistor_pin));
    }
    return NULL;
}

void *thread_decoder(void *arg) {
    (void)arg;

    bool start_bit_recieved = false;
    uint32_t last_pulse = 0;
    uint8_t received_bits = 0;
    uint8_t recv_buffer = 0;

    msg_init_queue(decoder_msg_queue, DECODER_MSG_QUEUE_SIZE);

    while(true) {
        msg_t msg;
        msg_receive(&msg);

        uint32_t pulse_timestamp = msg.content.value;

        printf("pulse: %ld\n", pulse_timestamp);

        if (!start_bit_recieved) {
            start_bit_recieved = true;
            ++received_bits;
        } else {
            uint32_t diff = pulse_timestamp - last_pulse;
            uint32_t approx_bit_count = round(diff / BIT_DURATION_US);
            uint32_t max_uncertainty = approx_bit_count * BIT_DURATION_TOLLERANCE;

            //check for timeout
            if (approx_bit_count > (uint32_t) (BITS_PER_FRAME - received_bits)) {
                received_bits = 0;
                start_bit_recieved = false;
                ++decode_error_cnt;
                continue;
            }

            if (diff >= approx_bit_count * BIT_DURATION_US + max_uncertainty
                    && diff <= approx_bit_count * BIT_DURATION_US + max_uncertainty
                    && approx_bit_count > 1) {
                for (size_t i = received_bits; i <= received_bits + approx_bit_count - 1; ++i) {
                    uint8_t bit_index = 8 - (i - 1);
                    if (i == BITS_PER_FRAME) {
                        continue;
                    }

                    recv_buffer |= 0x1 << bit_index;
                }

            } else {
                received_bits = 0;
                start_bit_recieved = false;
                ++decode_error_cnt;
                continue;
            }

            received_bits += approx_bit_count;
            if (received_bits == BITS_PER_FRAME) {
                msg_t send_msg;
                send_msg.content.value = received_bits;

                msg_send(&send_msg, print_thread_pid);
                received_bits = 0;
                start_bit_recieved = false;
            }
        }

        last_pulse = pulse_timestamp;
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

    gpio_init_int(transistor_pin, GPIO_IN_PD, GPIO_RISING, isr, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
