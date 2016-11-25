#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

//tock includes
#include "tock.h"
#include "console.h"
#include "timer.h"
#include "i2c_master_slave.h"
#include "gpio.h"
#include "storage_master.h"

#define BUFFER_SIZE 64

//i2c buffers
uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];

uint8_t request_buf[BUFFER_SIZE];
uint8_t return_buf[BUFFER_SIZE];

uint8_t reg = 0x00;
#define RPC_REQUEST 0x01
#define RPC_RETURN 0x02

static void i2c_master_slave_callback (
    int callback_type,
    int length,
    int unused __attribute__ ((unused)),
    void * callback_args __attribute__ ((unused))) {

    //for now only take writes
    if(callback_type == CB_SLAVE_READ_REQUEST) {
        //this is where you fulfill the rpc read from the edison
        if(reg == RPC_REQUEST) {
            //This allows the edison to read the most recent request
            memcpy(slave_read_buf,request_buf,BUFFER_SIZE);
        } else if(slave_write_buf[1] = RPC_RETURN) {
            //this allows the microcontroller to read the edison's response
            memcpy(slave_read_buf,return_buf,BUFFER_SIZE);
        }
        return;
    } else if (callback_type == CB_SLAVE_READ_COMPLETE) {
        return;
    } else if (callback_type == CB_SLAVE_WRITE) {
        reg = slave_write_buf[0];
        if(length > 1) {
            if(reg == RPC_REQUEST) {
                //this is an rpc request
                //
                //copy the rpc request to the request buffer
                memcpy(request_buf,slave_write_buf,length-1);

                //wakeup the edison to tell it that it has an rpc request
                storage_master_wakeup_edison();

            } else if(slave_write_buf[1] = RPC_RETURN) {
                //this is an rpc return from the edison
                //copy the rpc return to the return buffer
                memcpy(return_buf,slave_write_buf,length-1);

                //alert the module that the request has returned
            }
        }
    }
}

int main () {
    storage_master_enable_edison();

    //low configure i2c slave to listen
    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_slave_address(0x19);

    i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

    //listen
    i2c_master_slave_listen();
}
