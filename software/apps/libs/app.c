#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "message.h"
#include "protocol.h"
#include "module.h"
#include "app.h"

// Although message is designed to accept up to 2^15-1 byte messages,
// that's too big for our systems and we need to set a limit. I thought 4KB
// seemed reasonable
#define BUFSIZE 4096
uint8_t buf[BUFSIZE];

int app_send(uint8_t addr, uint8_t dest, uint8_t* key,
             uint8_t cmdrsp, uint8_t type,
             uint8_t func, int numarg, ...) {

    va_list valist;
    uint8_t arglen=0;
    size_t size=0;
    // cmdrsp + type + func + numargs (each of max 256)
    size_t buflen = sizeof(uint8_t)*3 + numarg*256;
    uint8_t buf[buflen];
    memset(buf, 0, buflen);
    // copy args to buffer
    buf[size++] = cmdrsp;
    buf[size++] = type;
    buf[size++] = func;
    va_start(valist, numarg);
    for(int i = 0; i<numarg; i+=2) {
        arglen = va_arg(valist, int);
        buf[size++] = arglen;
        memcpy(buf+size, va_arg(valist, uint8_t*), arglen);
        size+=arglen;
    }

    // testing
    //printf("(%d, %d) app buf:\n", size, buflen);
    //for(int i = 0; i < buflen; i++) {
    //    printf("%02x", buf[i]);
    //}
    //printf("\n");
    return protocol_send(addr, dest, key, buf, size);
}

int app_recv(uint8_t* key, uint8_t* cmdrsp, uint8_t* type, uint8_t* func, Arg* args, size_t* numargs) {
    uint8_t src;
    size_t i = 0;
    size_t len = message_recv(buf, BUFSIZE, &src);
    protocol_recv(buf, BUFSIZE, len, key, &len);
    //testing
    //for (int j = 0; j < len; j++) {
    //    printf("%02x", buf[j]);
    //}
    //printf("\n");

    if (len > BUFSIZE) return -1;
    *cmdrsp = buf[i++];
    *type   = buf[i++];
    *func   = buf[i++];
    *numargs = 0;
    while(i < len) {
        printf("i: %d, len: %d\n", i, len);
        //first byte is arglen
        uint8_t arglen = buf[i++];
        if (i + (size_t)arglen > BUFSIZE) return -2;
        args[*numargs].len = arglen;
        //following is arg
        memcpy(args[*numargs].arg, buf+i, arglen);
        *numargs+=1;
        i+=arglen;
    }

    return 0;
}
