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

#define INTERVAL_IN_MS 2000

static uint8_t key[32];
static uint8_t message[1024];

int main(void) {
    memset(key, 0, 32);
    strcpy((char *) key, "this is a key");
    printf("\n###\n\n\ntest app stack\n");

    while(1) {
        delay_ms(INTERVAL_IN_MS);
        memcpy(message, "hello there\0", strlen("hello there") + 1);
        signbus_io_init(SIGNBUS_TEST_SENDER_I2C_ADDRESS);
        signbus_app_send(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS,
                key, NotificationFrame, 0x00, 0x00, strlen("hello there") + 1, message);
        printf("SENDER: sent message\n");
    }
}
