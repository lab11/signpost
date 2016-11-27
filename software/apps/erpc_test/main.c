#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <console.h>
#include "test_arithmetic.h"
#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"


int main () {
    erpc_transport_t transport;
    transport = erpc_transport_i2c_master_slave_init(0x19,0x30);
    erpc_client_init(transport);

    delay_ms(5000);

    float result = 0;
    result =  add(5,6);

    if(result == 11) {
        putstr("RPC Completed Correctly\n");
    } else {
        putstr("RPC Failed\n");
    }
}
