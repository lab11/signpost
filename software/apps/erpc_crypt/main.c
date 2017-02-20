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
#include "signpost_api.h"


int main () {

    signpost_initialization_module_init(0x35,NULL);
    signpost_processing_init("~/path/to/sign.py");

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
