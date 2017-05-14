#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "led.h"
#include "timer.h"
#include "tock.h"

#include "signpost_api.h"
#include "signbus_app_layer.h"
#include "signbus_io_interface.h"

#ifndef MESSAGE_LENGTH
#define MESSAGE_LENGTH 1
#endif

#ifndef MODULE_ID
#define MODULE_ID 0x42
#endif

// This is a bit of a hack
uint8_t* signpost_api_addr_to_key(uint8_t addr);

int main(void) {
    printf("\n###\n\n\ni2c blast\n");

    led_on(0);

    int rc;
    do {
        rc = signpost_initialization_module_init(MODULE_ID, SIGNPOST_INITIALIZATION_NO_APIS);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);

    char send_buf[256];
    memset(send_buf, MODULE_ID, 256);

    led_off(0);

    // This is a bit hacky, but work around some stability issues a bit by not
    // blasting messages until everyone's initialized
    for (int i = 0; i < (2 * 10); i++) {
      if (i % 2) {
        led_on(0);
      } else {
        led_off(0);
      }
      delay_ms(500);
    }

    led_on(0);
    printf("BLASTOFF!\n");

    while(1) {
        rc = signbus_app_send(ModuleAddressController, signpost_api_addr_to_key,
                NotificationFrame, 0xbe, 0xef, MESSAGE_LENGTH, send_buf);
        if (rc < 0) {
            printf("Error sending (code: %d)\n", rc);
        }

        delay_ms(10);
        //printf("Sent a thing\n");
    }
}
