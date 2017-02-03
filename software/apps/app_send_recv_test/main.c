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

#define SENDER

Arg args[5];
uint8_t src;
unsigned char* test = "hello";
uint8_t vals[3];
uint8_t buf[1024];

static void cb(
        int callback_type __attribute__ ((unused)),
        int length,
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))) {
    printf("read %d from %02x: 0x", length, src);
    for(int i = 0; i < length; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}


int main() {
    uint8_t key[32];
    memset(key, 0, 32);
    strcpy(key, "this is a key");
    printf("\n###\n\n\ntest app stack\n");
    size_t numargs;

    while(1) {
        delay_ms(2000);
#ifdef SENDER
        args[0].len = 255;
        args[1].len = 50;
        strcpy(args[0].arg, "hello");
        strcpy(args[1].arg, "there");
        message_init(0x32);
        app_send(0x32, 0x18, key, 0x01, 0x00, 0x00, 2, args);
#else
        message_init(0x18);
        app_recv(key, vals, vals+1, vals+2, args, &numargs);
        printf("cmdrsp: %02x, type: %02x, func: %02x\n", vals[0], vals[1], vals[2]);
        for (int i = 0; i < numargs; i++) {
            printf("(%d) arg %d: %s", args[i].len, i, args[i].arg);
            printf("\n");
        }
#endif
    }
}
