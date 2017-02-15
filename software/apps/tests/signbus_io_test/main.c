#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "gpio.h"
#include "i2c_master_slave.h"
#include "signbus_io_interface.h"
#include "timer.h"
#include "tock.h"

//#define SENDER

int main (void) {


    uint8_t data[1024];

#ifdef SENDER
    signbus_io_init(0x25);
    i2c_master_slave_listen();

    //send some messages
    for(uint16_t i = 0; i < 1024; i++) {
        data[i] = i;
    }

    //a short message
    delay_ms(1000);
    signbus_io_send(0x19, data, 25);
    signbus_io_recv(data,1024,data);

    //a longer message
    delay_ms(1000);
    signbus_io_send(0x19, data, 1024);

    //a message that hits a data limit (single packet)
    delay_ms(1000);
    signbus_io_send(0x19, data, 247);

    //a message that hits a data limit (>1 packet)
    delay_ms(1000);
    signbus_io_send(0x19, data, 494);
#else

    for(uint16_t i = 0; i < 1024; i++) {
        data[i] = ((i % 256) & 0x00FF);
    }

    signbus_io_init(0x19);
    signbus_io_set_read_buffer(data,1024);

    while(1) {
        yield();
    }

    //receive and echo those messages
    uint8_t src;
    uint16_t len;
    len = signbus_io_recv(data,1024,&src);
    signbus_io_send(src,data,len);

    len = signbus_io_recv(data,1024,&src);
    signbus_io_send(src,data,len);

    len = signbus_io_recv(data,1024,&src);
    signbus_io_send(src,data,len);

    len = signbus_io_recv(data,1024,&src);
    signbus_io_send(src,data,len);
#endif



}
