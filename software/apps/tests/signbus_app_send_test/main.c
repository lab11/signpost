#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rng.h>
#include <timer.h>
#include <tock.h>

#include "signbus_app_layer.h"
#include "signbus_io_interface.h"
#include "signpost_entropy.h"

#define INTERVAL_IN_MS 2000


static uint8_t key[32];
static uint8_t* addr_to_key(uint8_t addr __attribute__((unused)) ) {
    memset(key, 0, 32);
    strcpy((char *) key, "this is a key");
    return key;
}

static uint8_t message[1024];

int main(void) {
    printf("\n###\n\n\ntest app stack\n");

    int rc;
    signpost_entropy_init();
    signbus_io_init(SIGNBUS_TEST_SENDER_I2C_ADDRESS);
    memcpy(message, "hello there\0", strlen("hello there") + 1);

    while(1) {
        delay_ms(INTERVAL_IN_MS);
        printf("SENDER: sending message\n");
        rc = signbus_app_send(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS,
                addr_to_key, NotificationFrame, 0xde, 0xad, strlen("hello there") + 1, message);
        if (rc < 0) {
            printf("signbus_app_send error %d\n", rc);
            continue;
        }
        printf("SENDER: sent message\n");
    }
}
