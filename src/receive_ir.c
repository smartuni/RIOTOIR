#include <stdio.h>
#include <string.h>

#include "msg.h"
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"

#include "crc8.h"

#include "receive_ir.h"
#include "ir_common.h"
#include "Riotoir_display.h"

#define DECODER_MSG_QUEUE_SIZE 128
#define PRINTER_MSG_QUEUE_SIZE 2048
#define MSG_BUFFER_SIZE 255

static msg_t decoder_msg_queue[DECODER_MSG_QUEUE_SIZE];
static msg_t printer_msg_queue[PRINTER_MSG_QUEUE_SIZE];

static char stack[THREAD_STACKSIZE_MAIN];
static char stack_decoder[THREAD_STACKSIZE_MAIN];
static unsigned int isr_counter = 0;
static unsigned int recv_error_cnt = 0;
static unsigned int decode_error_cnt = 0;

static gpio_t transistor_pin;

static kernel_pid_t decoder_thread_pid;
static kernel_pid_t print_thread_pid;


uint8_t recv_byte( void ) {
    msg_t msg;
    msg_receive(&msg);

    return msg.content.value;
}

/**
 * @brief recieves n bytes
 *
 * @param buff recieve buffer. Must be at least n bytes large.
 * @param n number of bytes to receive
 */
int recv_bytes( uint8_t* buff, uint8_t n ) {
    for (size_t i = 0; i < n; ++i) {
        //uint32_t start_timestamp = xtimer_now_usec();
        buff[i] = recv_byte();
    }
    return true;
}

void* thread_handler( void* arg ) {
    (void) arg;

    msg_init_queue(printer_msg_queue, PRINTER_MSG_QUEUE_SIZE);


    while (true) {
        uint8_t recv_buffer[MSG_BUFFER_SIZE];
        memset(recv_buffer, 0, MSG_BUFFER_SIZE);
        struct ir_package* msg = (struct ir_package*) recv_buffer;
        uint8_t bytes_received = 0;

        recv_bytes(recv_buffer, sizeof(struct ir_header) - bytes_received);
        bytes_received = sizeof(struct ir_header);

        if (msg->header.version != 0) {
            printf("Invalid package version: %d\n", msg->header.version);
            continue;
        }


        uint8_t expected_bytes = msg->header.length - sizeof(struct ir_header);

        recv_bytes(recv_buffer + sizeof(struct ir_header), expected_bytes);

        if (checkCRC8(msg->header.checksum, (uint8_t*) &( msg->header ), sizeof(struct ir_header) - 1) == false) {
            printf("Checksum invalid\n");
        } else {
            printf("msg received: %.250s\n", msg->msg);
            show_message((char *)msg->msg);
        }
    }
    return NULL;
}

void* thread_decoder( void* arg ) {
    (void) arg;

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

    while (true) {
        msg_t msg;
        msg_receive(&msg);

        uint32_t pulse_timestamp = msg.content.value;
        bool pin_high = msg.content.value & 0x1;

        uint32_t diff = pulse_timestamp - last_pulse;

        if (!pin_high && !start_bit_recieved && diff >= START_END_PULSE_US) {
            start_bit_recieved = true;

        } else if (!pin_high && start_bit_recieved && diff >= START_END_PULSE_US) {
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

void isr( void* arg ) {
    (void) arg;
    const uint32_t timestamp = xtimer_now_usec();

    msg_t msg;
    // encode pin state in lsb
    msg.content.value = ( timestamp & 0xFFFFFE ) | gpio_read(transistor_pin);

    if (msg_send(&msg, decoder_thread_pid) != 1) {
        ++recv_error_cnt;
    }

    ++isr_counter;
}

void setup_ir_recv( gpio_t ir_pin ) {
    transistor_pin = ir_pin;

    print_thread_pid = thread_create(
            stack, sizeof(stack),
            THREAD_PRIORITY_MAIN - 1,
            THREAD_CREATE_STACKTEST,
            thread_handler, NULL,
            "ir_printer");

    decoder_thread_pid = thread_create(
            stack_decoder, sizeof(stack_decoder),
            THREAD_PRIORITY_MAIN - 2,
            THREAD_CREATE_STACKTEST,
            thread_decoder, NULL,
            "ir_decoder");


    gpio_init_int(transistor_pin, GPIO_IN_PD, GPIO_BOTH, isr, NULL);
}

