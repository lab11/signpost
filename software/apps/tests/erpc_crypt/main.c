#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gpio.h>
#include <timer.h>
#include <tock.h>

#include "signpost_api.h"
#include "test_crypt.h"


int main (void) {
    int rc;

    do {
        rc = signpost_initialization_module_init(0x35,NULL);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);

    rc = signpost_processing_init("~/path/to/sign.py");
    if (rc < 0) {
        printf(" - Error initializing signpost processing. Quitting.\n");
        return rc;
    }

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
