#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <console.h>
#include "test_crypt.h"
#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"



int main () {
    erpc_transport_t transport;
    transport = erpc_transport_i2c_master_slave_init(0x19,0x30);
    erpc_client_init(transport);

    ret* signed_data;
    uint8_t data[100];

    signed_data = sign(data,10);

    gpio_set(10);
    gpio_set(11);

    for(uint8_t i = 0; i < signed_data->len; i++) {
        if(signed_data->data[i] == 1) {

        } else {
            gpio_clear(10);
            gpio_clear(11);
        }
    }

}
