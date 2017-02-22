#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timer.h"
#include "tock.h"

#include "signpost_api.h"
#include "signbus_io_interface.h"

#define INTERVAL_IN_MS 2000


int main(void) {
    printf("\n###\n\n\ntest app init\n");

    int rc;
    do {
        rc = signpost_initialization_module_init(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);

    int i = 0;
    while(1) {
        delay_ms(INTERVAL_IN_MS);
        printf("doin' stuff %d\n", i++);
    }
}
