#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "gpio.h"
#include "signbus_io_interface.h"
#include "timer.h"
#include "tock.h"

int main (void) {
    uint8_t data[1024];

    for(uint16_t i = 0; i < 1024; i++) {
        data[i] = ((i % 256) & 0x00FF);
    }

    signbus_io_init(0x19);
    signbus_io_set_read_buffer(data,1024);

    // n.b. this while loop was here when I split
    // the send and recv test apps, I assume that
    // neal had a purpose for it / the uncalled
    // functions below it -Pat

    while(1) {
        yield();
    }

    //receive and echo those messages
    uint8_t src;
    uint16_t len;
    bool encrypted;
    len = signbus_io_recv(1024,data,&encrypted,&src);
    signbus_io_send(src,0,data,len);

    len = signbus_io_recv(1024,data,&encrypted,&src);
    signbus_io_send(src,0,data,len);

    len = signbus_io_recv(1024,data,&encrypted,&src);
    signbus_io_send(src,0,data,len);

    len = signbus_io_recv(1024,data,&encrypted,&src);
    signbus_io_send(src,0,data,len);
}
