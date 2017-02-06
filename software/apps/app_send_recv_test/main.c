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

static Arg args[5];
static uint8_t vals[3];
static size_t numargs;
static uint8_t key[32];

static void cb(size_t length){
    printf("cmdrsp: %02x, type: %02x, func: %02x\n", vals[0], vals[1], vals[2]);
    for (int i = 0; i < numargs; i++) {
        printf("(%d) arg %d: %s", args[i].len, i, args[i].arg);
        printf("\n");
    }
    printf("\n");
    app_recv_async(cb, key, vals, vals+1, vals+2, &numargs, args);
}


int main() {
    memset(key, 0, 32);
    strcpy(key, "this is a key");
    printf("\n###\n\n\ntest app stack\n");

#ifndef SENDER
    message_init(0x18);
    app_recv_async(cb, key, vals, vals+1, vals+2, &numargs, args);
#endif
    while(1) {
#ifdef SENDER
        delay_ms(2000);
        args[0].len = 255;
        args[1].len = 50;
        strcpy(args[0].arg, "hello");
        strcpy(args[1].arg, "there");
        message_init(0x32);
        app_send(0x18, key, 0x01, 0x00, 0x00, 2, args);
#else
        delay_ms(500);
        printf("doing stuff\n");
#endif
    }
}
