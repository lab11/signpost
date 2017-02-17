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
#include "rng.h"
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

    signpost_entropy_init();
    signbus_io_init(SIGNBUS_TEST_SENDER_I2C_ADDRESS);
    memcpy(message, "hello there\0", strlen("hello there") + 1);

    // XXX temp until neal changes send api
    uint8_t* key_FIXME = addr_to_key(0);

    while(1) {
        delay_ms(INTERVAL_IN_MS);
        printf("SENDER: sending message\n");
        signbus_app_send(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS,
                key_FIXME, NotificationFrame, 0xde, 0xad, strlen("hello there") + 1, message);
        printf("SENDER: sent message\n");
    }
}
