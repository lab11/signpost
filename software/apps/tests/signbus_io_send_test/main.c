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

int main (void) {
    uint8_t data[1024];

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
}
