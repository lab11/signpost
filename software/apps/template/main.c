#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "tock.h"
#include "console.h"
#include "timer.h"
#include "signpost_api.h"
#include "signbus_io_interface.h"

#define I2C_ADDRESS 0x15

int main(void) {
    // initialize app with
    signpost_initialization_module_init(I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);

    while(true) {
        /* YOUR CODE GOES HERE */

        delay_ms(1000);
    }
}
