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

#define INTERVAL_IN_MS 2000


int main(void) {
    printf("\n###\n\n\ntest app init\n");
    signpost_initialization_module_init(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);

    int i = 0;
    while(1) {
        delay_ms(INTERVAL_IN_MS);
        printf("doin' stuff %d\n", i++);
    }
}
