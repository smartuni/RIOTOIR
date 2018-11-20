//
// Created by exelend on 20.11.18.
//

#include <thread.h>

typedef int bool;
#define true 1
#define false 0

int main(void){

    uint8_t send_payload[] = "Return to sender!";

    uint8_t rcv_payload[sizeof(send_payload)];

    // Start recive-thread
    // TODO Pointer auf Rcv-function, rcv name
    char rcv_thread_stack[THREAD_STACKSIZE_MAIN];
    kernel_pid_t rcv_Thread = thread_create(rcv_thread_stack, sizeof(rcv_thread_stack), THREAD_PRIORITY_MAIN -1, THREAD_CREATE_STACKTEST, pointer_auf_recive, rcv_payload, "empfängername" );

    // TODO send message xxx

    sprintf("Testresult:\n\nSend:\nPayload:  %s\n\nRecieved: %s\n\n", send_payload, rcv_payload);

    bool len_ok = true

    for(int i = 0; i < sizeof(send_payload); i++){
        if(send_payload[i] != rcv_payload[i]){
            len_ok = false;
            sprintf("Test -> FAIL\n");
            return;
        }
    }

    sprintf("Test -> successfully\n")

}