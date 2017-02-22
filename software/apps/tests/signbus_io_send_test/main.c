#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gpio.h>
#include <i2c_master_slave.h>
#include <timer.h>
#include <tock.h>

#include "signbus_io_interface.h"

int main (void) {
    uint8_t data[1024];
    uint8_t src;
    bool enc;

    signbus_io_init(0x25);
    i2c_master_slave_listen();

    //send some messages
    for(uint16_t i = 0; i < 1024; i++) {
        data[i] = i;
    }

    //a short message
    delay_ms(1000);
    signbus_io_send(0x19, 0, data, 25);
    signbus_io_recv(1024, data, &enc, &src);

    //a longer message
    delay_ms(1000);
    signbus_io_send(0x19, 0, data, 1024);

    //a message that hits a data limit (single packet)
    delay_ms(1000);
    signbus_io_send(0x19, 0, data, 247);

    //a message that hits a data limit (>1 packet)
    delay_ms(1000);
    signbus_io_send(0x19, 0, data, 494);
}
