#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "message.h"
#include "protocol.h"
#include "module.h"
#include "app.h"

int app_send(uint8_t addr, uint8_t dest, uint8_t* key,
             uint8_t cmdrsp, uint8_t type,
             uint8_t func, int numarg, ...) {

    va_list valist;
    uint8_t arglen=0;
    size_t size=0;
    // cmdrsp + type + func + numargs (each of max 256)
    size_t buflen = 8*3 + numarg*256;
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
        memcpy(buf+size, (uint8_t*)va_arg(valist, int), arglen);
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

//int app_recv() {
//
//    return 0;
//}
