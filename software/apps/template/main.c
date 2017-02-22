#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <timer.h>
#include <tock.h>

#include "signpost_api.h"

#define I2C_ADDRESS 0x15

int main(void) {
    int rc;

    // Before an app can do anything useful, it needs to announce itself to the
    // signpost, negotiate keys, and learn what other services are available on
    // this signpost:
    do {
        // The first parameter is the address of this module.
        // The second parameter lists the services that this module _exports_ to other modules.
        rc = signpost_initialization_module_init(I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);


    while(true) {
        /* YOUR CODE GOES HERE */

        delay_ms(1000);
    }
}
