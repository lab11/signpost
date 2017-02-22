#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

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
    EdisonApiType = 7,
    HighestApiType = EdisonApiType,
} signbus_api_type_t;

/// Blocking method to send a message
/// Returns < 0 on failure.
__attribute__((warn_unused_result))
int signbus_app_send(
        uint8_t dest,                       // I2C address of destination
        uint8_t* (*addr_to_key)(uint8_t),   // Translation function from address -> key
        signbus_frame_type_t frame_type,    // Frame Type
        signbus_api_type_t api_type,        // Which API?
        uint8_t message_type,               // Which API method?
        size_t message_length,              // How many bytes from message param to send
        const uint8_t* message              // Buffer to send from
        );

/// Blocking method to receive a message
/// Returns < 0 on failure.
__attribute__((warn_unused_result))
int signbus_app_recv(
        uint8_t *sender_address,            // I2C address of sender
        uint8_t* (*addr_to_key)(uint8_t),   // Translation function from address -> key
        signbus_frame_type_t* frame_type,   // Frame Type
        signbus_api_type_t* api_type,       // Which API?
        uint8_t* message_type,              // Which API method?
        size_t* message_length,             // How many bytes in message param are valid
        uint8_t **message,                  // Pointer to beginnig of message
        size_t recv_buflen,                 // Size of recv buffer
        uint8_t* recv_buf                   // Buffer to recieve message into
        );

/// Parameter matches return of sync
typedef void (signbus_app_callback_t)(int);

/// Non-blocking method to receive a message
/// All parameters must remain valid until the callback method executes
/// Returns < 0 on failure.
__attribute__((warn_unused_result))
int signbus_app_recv_async(
        signbus_app_callback_t callback,    // Function to call when message received
        uint8_t *sender_address,            // I2C address of sender
        uint8_t* (*addr_to_key)(uint8_t),   // Translation function from address -> key
        signbus_frame_type_t* frame_type,   // Frame Type
        signbus_api_type_t* api_type,       // Which API?
        uint8_t* message_type,              // Which API method?
        size_t* message_length,             // How many bytes in message param are valid
        uint8_t **message,                  // Pointer to beginnig of message
        size_t recv_buflen,                 // Size of recv buffer
        uint8_t* recv_buf                   // Buffer to recieve message into
        );
