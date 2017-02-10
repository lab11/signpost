#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Although underlying layers designed to accept up to 2^15-1 byte messages,
// that's too big for our systems and we need to set a limit. I thought 4KB
// seemed reasonable
#define BUFSIZE 4096

typedef void (app_cb)(size_t);

int app_send(uint8_t dest, uint8_t* key,
             uint8_t frame_type, uint8_t api_type,
             uint8_t message_type, size_t message_length, uint8_t* message);


int app_recv(uint8_t* key,
        uint8_t* frame_type, uint8_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message);

int app_recv_async(app_cb cb, uint8_t* key,
                   uint8_t* frame_type, uint8_t* api_type, uint8_t* message_type,
                   size_t* message_length, uint8_t* message);
