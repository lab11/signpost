#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Although underlying layers designed to accept up to 2^15-1 byte messages,
// that's too big for our systems and we need to set a limit. I thought 4KB
// seemed reasonable
#define BUFSIZE 4096

typedef void (signbus_app_callback_t)(size_t);

typedef enum signbus_frame_type {
        NotificationFrame = 0,
        CommandFrame = 1,
        ResponseFrame = 2,
        ErrorFrame = 3,
} signbus_frame_type_t;

typedef enum signbus_api_type {
        InitializationApiType = 1,
        StorageApiType = 2,
        NetworkingApiType = 3,
        ProcessingApiType = 4,
        EnergyApiType = 5,
        TimeLocationApiType = 6,
        HighestApiType = TimeLocationApiType,
} signbus_api_type_t;

int signbus_app_send(uint8_t dest, uint8_t* key,
        signbus_frame_type_t frame_type, signbus_api_type_t api_type, uint8_t message_type,
        size_t message_length, uint8_t* message);


int signbus_app_recv(uint8_t* key,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message);

int signbus_app_recv_async(signbus_app_callback_t cb, uint8_t* key,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message);
