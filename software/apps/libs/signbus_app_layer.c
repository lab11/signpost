#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "signbus_app_layer.h"
#include "signbus_io_interface.h"
#include "signbus_protocol_layer.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

typedef struct {
    uint8_t* src;
    uint8_t* len;
    uint8_t* sender_address;
    uint8_t* key;
    signbus_frame_type_t* frame_type;
    signbus_api_type_t* api_type;
    uint8_t* message_type;
    size_t* message_length;
    uint8_t* message;
    size_t message_buflen;
    signbus_app_callback_t* cb;
} app_cb_data;

static uint8_t app_buf[BUFSIZE];
static app_cb_data cb_data;

static int app_parse(uint8_t* to_parse, size_t len,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message, size_t message_buflen) {
    size_t i = 0;
    if (len > BUFSIZE) return -1;
    *frame_type     = to_parse[i++];
    *api_type       = to_parse[i++];
    *message_type   = to_parse[i++];
    *message_length = len - 3;
    if (*message_length > message_buflen) return ESIZE;
    memcpy(message, to_parse + 3, *message_length);

    return 0;
}

static void app_layer_callback(size_t len) {
    int rc;
    rc = app_parse(app_buf, len,
            cb_data.frame_type, cb_data.api_type, cb_data.message_type,
            cb_data.message_length, cb_data.message, cb_data.message_buflen);
    cb_data.cb(rc);
}

int signbus_app_send(uint8_t dest, uint8_t* key,
        signbus_frame_type_t frame_type, signbus_api_type_t api_type, uint8_t message_type,
        size_t message_length, uint8_t* message) {
    size_t payload_length = 1 + 1 + 1 + message_length;
    if (payload_length > BUFSIZE) {
        return ESIZE;
    }

    uint8_t payload[payload_length];

    // copy args to buffer
    payload[0] = frame_type;
    payload[1] = api_type;
    payload[2] = message_type;

    memcpy(payload + 3, message, message_length);
    return signbus_protocol_send(dest, key, payload, payload_length);
}

int signbus_app_recv(uint8_t* sender_address, uint8_t* key,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message, size_t message_buflen) {
    size_t len = signbus_io_recv(app_buf, BUFSIZE, sender_address);

    len = signbus_protocol_recv(app_buf, BUFSIZE, len, key);

    int rc;
    rc = app_parse(app_buf, len, frame_type, api_type, message_type, message_length, message, message_buflen);

    return rc;
}

int signbus_app_recv_async(signbus_app_callback_t cb, uint8_t* sender_address, uint8_t* key,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message, size_t message_buflen) {
    cb_data.sender_address = sender_address;
    cb_data.key = key;
    cb_data.frame_type = frame_type;
    cb_data.api_type = api_type;
    cb_data.message_type = message_type;
    cb_data.message_length = message_length;
    cb_data.message = message;
    cb_data.message_buflen = message_buflen;
    cb_data.cb = cb;

    return signbus_protocol_recv_async(app_layer_callback, app_buf, BUFSIZE, key);
}
