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
#include "protocol.h"
#include "message.h"
#include "app.h"
#include "rng.h"

//#define SENDER

static uint8_t key[32];

static uint8_t frame_type, api_type, message_type;
static size_t message_length;
static uint8_t message[1024];

static void cb(size_t length) {
    // Print data for this received message
    printf("frame_type: %02x, api_type: %02x, message_type: %02x, message_len: %d, %02x\n",
            frame_type, api_type, message_type, message_length, message_length);
    if ((frame_type == 0x01) && (api_type == 0x00) && (message_type == 0x00)) {
        printf("message: >>>%s<<<\n", message);
    } else {
        printf("unknown message: 0x");
        for (size_t i = 0; i < message_length; i++) {
            printf("%02x", message[i]);
        }
        printf("\n");
    }

    // Listen for another message (re-using the same buffers)
    app_recv_async(cb, key,
            &frame_type, &api_type, &message_type,
            &message_length, message);
}


int main() {
    memset(key, 0, 32);
    strcpy(key, "this is a key");
    printf("\n###\n\n\ntest app stack\n");

#ifndef SENDER
    printf("RECEIVER: Begin listening\n\n");

    message_init(0x18);
    app_recv_async(cb, key,
            &frame_type, &api_type, &message_type,
            &message_length, message);
#endif
    while(1) {
#ifdef SENDER
        delay_ms(2000);
        memcpy(message, "hello there\0", strlen("hello there") + 1);
        message_init(0x32);
        app_send(0x18, key, 0x01, 0x00, 0x00, strlen("hello there") + 1, message);
        printf("SENDER: sent message\n");
#else
        delay_ms(500);
        printf("RECEIVER: doing stuff\n");
#endif
    }
}
