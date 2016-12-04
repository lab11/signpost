#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

//tock includes
#include "tock.h"
#include "console.h"
#include "message.h"
#include "gpio.h"
#include "storage_master.h"

#define BUFFER_SIZE 2048

static uint8_t request_buf[BUFFER_SIZE];
static uint8_t return_buf[BUFFER_SIZE];
static uint8_t message_buf[BUFFER_SIZE];
static uint8_t return_address;

static uint8_t rpc_pending = 0;

#define RPC_REQUEST 0x01
#define RPC_RETURN 0x02

int main () {
    message_init(0x19);
    storage_master_enable_edison();
    gpio_enable_output(2);
    gpio_set(2);

    while(1) {
        uint8_t src;
        uint32_t len;
        len = message_recv(message_buf, BUFFER_SIZE, &src);

        if(message_buf[0] == RPC_REQUEST) {
            if(!rpc_pending) {
                gpio_clear(2);
                rpc_pending = 1;
                memcpy(request_buf, message_buf+1,len-1);
                message_set_read_buffer(request_buf,len-1);
                return_address = src;
                storage_master_wakeup_edison();
            }

        } else if(message_buf[0] == RPC_RETURN) {
            gpio_set(2);
            memcpy(return_buf, message_buf+1,len-1);
            rpc_pending = 0;
            message_send(return_address, return_buf, len-1);
        }
    }
}

