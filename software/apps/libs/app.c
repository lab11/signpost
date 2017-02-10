#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "message.h"
#include "protocol.h"
#include "module.h"
#include "app.h"

typedef struct {
    uint8_t* src;
    uint8_t* len;
    uint8_t* key;
    uint8_t* frame_type;
    uint8_t* api_type;
    uint8_t* message_type;
    size_t* message_length;
    uint8_t* message;
    app_cb* cb;
} app_cb_data;

static uint8_t app_buf[BUFSIZE];
static app_cb_data cb_data;

static int app_parse(uint8_t* to_parse, size_t len,
        uint8_t* frame_type, uint8_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message) {
    size_t i = 0;
    if (len > BUFSIZE) return -1;
    *frame_type     = to_parse[i++];
    *api_type       = to_parse[i++];
    *message_type   = to_parse[i++];
    *message_length = len - 3;
    memcpy(message, to_parse + 3, *message_length);

    return 0;
}

static void app_callback(size_t len) {
    app_parse(app_buf, len,
            cb_data.frame_type, cb_data.api_type, cb_data.message_type,
            cb_data.message_length, cb_data.message);
    cb_data.cb(SUCCESS);
}

int app_send(uint8_t dest, uint8_t* key,
             uint8_t frame_type, uint8_t api_type, uint8_t message_type,
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
    return protocol_send(dest, key, payload, payload_length);
}

int app_recv(uint8_t* key,
        uint8_t* frame_type, uint8_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message) {
    uint8_t src;
    size_t len = message_recv(app_buf, BUFSIZE, &src);

    len = protocol_recv(app_buf, BUFSIZE, len, key);
    app_parse(app_buf, len, frame_type, api_type, message_type, message_length, message);

    return 0;
}

int app_recv_async(app_cb cb, uint8_t* key,
        uint8_t* frame_type, uint8_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message) {
    cb_data.key = key;
    cb_data.frame_type = frame_type;
    cb_data.api_type = api_type;
    cb_data.message_type = message_type;
    cb_data.message_length = message_length;
    cb_data.message = message;
    cb_data.cb = cb;

    return protocol_recv_async(app_callback, app_buf, BUFSIZE, key);
}
