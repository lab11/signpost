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

#define INTERVAL_IN_MS 2000

//#define SENDER

static uint8_t key[32];

static frame_type_t frame_type;
static uint8_t api_type;
static uint8_t message_type;
static size_t message_length;
static uint8_t message[1024];

static bool recent_message = false;

static void cb(size_t length) {
    // Print data for this received message
    printf("frame_type: %02x, api_type: %02x, message_type: %02x, message_len: %d, %02x\n",
            frame_type, api_type, message_type, message_length, message_length);
    if ((frame_type == NotificationFrame) && (api_type == 0x00) && (message_type == 0x00)) {
        printf("message: >>>%s<<<\n", message);
    } else {
        printf("unknown message: 0x");
        for (size_t i = 0; i < message_length; i++) {
            printf("%02x", message[i]);
        }
        printf("\n");
    }

    // Tickle our pseduo-watchdog
    recent_message = true;

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
        delay_ms(INTERVAL_IN_MS);
#ifdef SENDER
        memcpy(message, "hello there\0", strlen("hello there") + 1);
        message_init(0x32);
        app_send(0x18, key, NotificationFrame, 0x00, 0x00, strlen("hello there") + 1, message);
        printf("SENDER: sent message\n");
#else
        if (recent_message == false) {
            printf("RECEIVER: No message in %d ms\n", INTERVAL_IN_MS);
        } else {
            recent_message = false;
        }
#endif
    }
}
