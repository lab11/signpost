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
uint8_t return_address;

uint8_t reg = 0x00;
#define RPC_REQUEST 0x01
#define RPC_RETURN 0x02
#define RPC_RETURN_ADDRESS 0x03

//Test procedure.
//1) turn on signpost
//2) another module writes data to 0x19
//3) I see this occur on my salae
//4) I also observe that the callback is getting called because the edison
//wakeup pin works as expected
//5) I read reg 0x01 from address 0x19
//6) If the commented out lines in main are there it returns that data
//      If the lines are commented out it returns all 0s

static void i2c_master_slave_callback (
    int callback_type,
    int length,
    int unused __attribute__ ((unused)),
    void * callback_args __attribute__ ((unused))) {

    if(callback_type == CB_SLAVE_READ_REQUEST) {
        //this isn't really working right now
        //i2c_master_slave_read_ready(BUFFER_SIZE);
        putstr("read request");
        return;
    } else if (callback_type == CB_SLAVE_READ_COMPLETE) {
        //putstr("read complete");
        return;
    } else if (callback_type == CB_SLAVE_WRITE) {
        reg = slave_write_buf[0];
        if(length > 1) {
            //assume this is a real request
            if(reg == RPC_REQUEST) {
                //this is an rpc request
                //
                //copy the rpc request to the request buffer
                memcpy(request_buf,slave_write_buf+1,BUFFER_SIZE-1);
                memcpy(slave_read_buf,request_buf,BUFFER_SIZE);
                i2c_master_slave_read_ready(BUFFER_SIZE);

                //I know that this write is occuring. I am observing
                //the edison wakeup pin
                //wakeup the edison to tell it that it has an rpc request
                storage_master_wakeup_edison();

            } else if(reg == RPC_RETURN) {
                //this is an rpc return from the edison
                //copy the rpc return to the return buffer
                memcpy(return_buf,slave_write_buf+1,length-1);
                memcpy(slave_read_buf,return_buf,BUFFER_SIZE);
                i2c_master_slave_read_ready(BUFFER_SIZE);

                //alert the module that the request has returned
                //should this happen over i2c?? I would like to just set the
                //alert pin but that's on another microcontroller...
            } else if(reg == RPC_RETURN_ADDRESS) {
                memcpy(&return_address,slave_write_buf+1,1);
                //memcpy(slave_read_buf,&return_address,BUFFER_SIZE);
            }
        } else {
            //just setting the register - asume it's to read it soon
            if(reg == RPC_REQUEST) {
                memcpy(slave_read_buf,request_buf,BUFFER_SIZE);
                i2c_master_slave_read_ready(BUFFER_SIZE);
            } else if(reg == RPC_RETURN) {
                memcpy(slave_read_buf,return_buf,BUFFER_SIZE);
                i2c_master_slave_read_ready(BUFFER_SIZE);
            } else if(reg == RPC_RETURN_ADDRESS) {
                //no one should ever read this
            }
        }
    }
}

int main () {
    storage_master_enable_edison();

    //If these lines are here the read works and returns this data
    /*slave_read_buf[0] = 0x10;
    slave_read_buf[1] = 0x11;
    slave_read_buf[2] = 0x12;*/

    i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

    //low configure i2c slave to listen
    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_slave_address(0x19);

    //listen
    i2c_master_slave_listen();

    while(1) {
        yield();
        i2c_master_slave_read_ready(BUFFER_SIZE);
    }
}
