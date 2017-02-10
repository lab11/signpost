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
    uint8_t* reason;
    uint8_t* type;
    uint8_t* func;
    size_t* numarg;
    Arg* args;
    app_cb* cb;
} app_cb_data;

uint8_t buf[BUFSIZE];
static app_cb_data cb_data;

int app_parse(uint8_t* buf, size_t len, uint8_t* reason, uint8_t* type, uint8_t* func, size_t* numarg, Arg* args);

void app_callback(size_t len) {
    app_parse(buf, len, cb_data.reason, cb_data.type, cb_data.func, cb_data.numarg, cb_data.args);
    cb_data.cb(0);
}

int app_send(uint8_t dest, uint8_t* key,
             uint8_t reason, uint8_t type,
             uint8_t func, int numarg, Arg* args) {

    size_t size=0;
    // reason + type + func + numargs (each of max 256)
    size_t len = sizeof(uint8_t)*3 + numarg*256;
    // len is too large
    if(len > BUFSIZE) return -1;
    uint8_t buf[len];
    memset(buf, 0, len);
    // copy args to buffer
    buf[size++] = reason;
    buf[size++] = type;
    buf[size++] = func;
    for(int i = 0; i<numarg; i+=1) {
        buf[size++] = args[i].len;
        memcpy(buf+size, args[i].arg, args[i].len);
        size+=args[i].len;
    }
    return protocol_send(dest, key, buf, size);
}

int app_parse(uint8_t* buf, size_t len, uint8_t* reason, uint8_t* type, uint8_t* func, size_t* numarg, Arg* args) {
    size_t i = 0;
    if (len > BUFSIZE) return -1;
    *reason = buf[i++];
    *type   = buf[i++];
    *func   = buf[i++];
    *numarg = 0;
    while(i < len) {
        //first byte is arglen
        uint8_t arglen = buf[i++];
        if (i + (size_t)arglen > BUFSIZE) return -2;
        args[*numarg].len = arglen;
        //following is arg
        memcpy(args[*numarg].arg, buf+i, arglen);
        *numarg+=1;
        i+=arglen;
    }

    return 0;
}

int app_recv(uint8_t* key, uint8_t* reason, uint8_t* type, uint8_t* func, size_t* numarg, Arg* args) {
    uint8_t src;
    size_t len = message_recv(buf, BUFSIZE, &src);

    len = protocol_recv(buf, BUFSIZE, len, key);
    app_parse(buf, len, reason, type, func, numarg, args);

    return 0;
}

int app_recv_async(app_cb cb, uint8_t* key, uint8_t* reason, uint8_t* type, uint8_t* func, size_t* numarg, Arg* args) {
    cb_data.key = key;
    cb_data.reason = reason;
    cb_data.type = type;
    cb_data.func = func;
    cb_data.numarg = numarg;
    cb_data.args = args;
    cb_data.cb = cb;

    return protocol_recv_async(app_callback, buf, BUFSIZE, key);
}
