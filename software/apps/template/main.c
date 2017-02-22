#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "signbus_io_interface.h"
#include "signpost_api.h"
#include "timer.h"
#include "tock.h"

#define I2C_ADDRESS 0x15

int main(void) {
    // initialize app with
    signpost_initialization_module_init(I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);

    while(true) {
        /* YOUR CODE GOES HERE */

        delay_ms(1000);
    }
}
