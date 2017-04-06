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
#include "signbus_app_layer.h"
#include "signbus_io_interface.h"

#ifndef MESSAGE_LENGTH
#define MESSAGE_LENGTH 1
#endif

#ifndef MODULE_ID
#define MODULE_ID 1
#endif

// This is a bit of a hack
uint8_t* signpost_api_addr_to_key(uint8_t addr);

int main(void) {
    printf("\n###\n\n\ni2c blast\n");

    int rc;
    do {
        rc = signpost_initialization_module_init(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS, SIGNPOST_INITIALIZATION_NO_APIS);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);

    char send_buf[256];
    memset(send_buf, MODULE_ID, 256);

    while(1) {
        rc = signbus_app_send(ModuleAddressController, signpost_api_addr_to_key,
                NotificationFrame, 0, 0, MESSAGE_LENGTH, send_buf);
        if (rc < 0) {
            printf("Error sending (code: %d)\n", rc);
        }

        //printf("Sent a thing\n");
    }
}
