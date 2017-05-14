#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "signbus_app_layer.h"
#include "signbus_io_interface.h"
#include "signbus_protocol_layer.h"

#pragma GCC diagnostic ignored "-Wstack-usage="

typedef struct {
    signbus_app_callback_t* cb;
    uint8_t* sender_address;
    signbus_frame_type_t* frame_type;
    signbus_api_type_t* api_type;
    uint8_t* message_type;
    size_t* message_length;
    uint8_t** message;
    size_t recv_buflen;
    uint8_t* recv_buf;
} app_cb_data;

static app_cb_data cb_data;

static int app_parse(uint8_t* recv_buf, size_t received_length,
        signbus_frame_type_t* frame_type, signbus_api_type_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t** message) {
    *frame_type     = recv_buf[0];
    *api_type       = recv_buf[1];
    *message_type   = recv_buf[2];
    *message_length = received_length - 3;
    *message        = recv_buf + 3;

    return *message_length;
}

int signbus_app_send(uint8_t dest, uint8_t* (*addr_to_key)(uint8_t),
        signbus_frame_type_t frame_type, signbus_api_type_t api_type, uint8_t message_type,
        size_t message_length, const uint8_t* message) {
    size_t payload_length = 1 + 1 + 1 + message_length;
    uint8_t payload[payload_length];

    SIGNBUS_DEBUG("dest %02x key -- fr %02x api %02x msg %02x msg_len %d msg %p\n",
            dest, frame_type, api_type, message_type, message_length, message);

    // copy args to buffer
    payload[0] = frame_type;
    payload[1] = api_type;
    payload[2] = message_type;

    memcpy(payload + 3, message, message_length);
    return signbus_protocol_send(dest, addr_to_key, payload, payload_length);
}

int signbus_app_recv(
        uint8_t* sender_address,
        uint8_t* (*addr_to_key)(uint8_t),
        signbus_frame_type_t* frame_type,
        signbus_api_type_t* api_type,
        uint8_t* message_type,
        size_t* message_length,
        uint8_t** message,
        size_t recv_buflen,
        uint8_t* recv_buf
        ) {
    int len_or_rc;

    len_or_rc = signbus_protocol_recv(sender_address, addr_to_key, recv_buflen, recv_buf);
    if (len_or_rc < 0) return len_or_rc;

    len_or_rc = app_parse(recv_buf, len_or_rc,
            frame_type, api_type, message_type,
            message_length, message);

    return len_or_rc;
}

static void app_layer_callback(int len_or_rc) {
    SIGNBUS_DEBUG("len_or_rc %d\n", len_or_rc);
    if (len_or_rc < 0) return cb_data.cb(len_or_rc);

    len_or_rc = app_parse(cb_data.recv_buf, len_or_rc,
            cb_data.frame_type, cb_data.api_type, cb_data.message_type,
            cb_data.message_length, cb_data.message);
    cb_data.cb(len_or_rc);
}

int signbus_app_recv_async(
        signbus_app_callback_t cb,
        uint8_t* sender_address,
        uint8_t* (*addr_to_key)(uint8_t),
        signbus_frame_type_t* frame_type,
        signbus_api_type_t* api_type,
        uint8_t* message_type,
        size_t* message_length,
        uint8_t** message,
        size_t recv_buflen,
        uint8_t* recv_buf
        ) {
    cb_data.cb = cb;
    cb_data.sender_address = sender_address;
    cb_data.frame_type = frame_type;
    cb_data.api_type = api_type;
    cb_data.message_type = message_type;
    cb_data.message_length = message_length;
    cb_data.message = message;
    cb_data.recv_buflen = recv_buflen;
    cb_data.recv_buf = recv_buf;

    return signbus_protocol_recv_async(app_layer_callback,
            sender_address, addr_to_key, recv_buflen, recv_buf);
}
