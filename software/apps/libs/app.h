#pragma once

#include <stdarg.h>

// Although underlying layers designed to accept up to 2^15-1 byte messages,
// that's too big for our systems and we need to set a limit. I thought 4KB
// seemed reasonable
#define BUFSIZE 4096

typedef struct {
    uint8_t len;
    uint8_t arg[256];
} Arg;

typedef void (app_cb)(size_t);

int app_send(uint8_t dest, uint8_t* key,
             uint8_t reason, uint8_t type,
             uint8_t func, int numarg, Arg* args);


int app_recv(uint8_t* key, uint8_t* reason, uint8_t* type,
             uint8_t* func, size_t* numarg, Arg* args);

int app_recv_async(app_cb cb, uint8_t* key, uint8_t* reason,
                   uint8_t* type, uint8_t* func,
                   size_t* numarg, Arg* args);
