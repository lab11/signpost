#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gpio.h>
#include <timer.h>
#include <tock.h>

#include "signbus_io_interface.h"

int main (void) {
    uint8_t data[1024];
    int rc;

    for(uint16_t i = 0; i < 1024; i++) {
        data[i] = ((i % 256) & 0x00FF);
    }

    signbus_io_init(0x19);
    rc = signbus_io_set_read_buffer(data,1024);
    if (rc < 0) {
        printf("%d: signbus_io_set_read_buffer error %d\n", __LINE__, rc);
        return rc;
    }

    // n.b. this while loop was here when I split
    // the send and recv test apps, I assume that
    // neal had a purpose for it / the uncalled
    // functions below it -Pat

    while(1) {
        yield();
    }

    //receive and echo those messages
    uint8_t src;
    int len;
    bool encrypted;
    len = signbus_io_recv(1024,data,&encrypted,&src);
    if (len < 0) {
        printf("%d: signbus_io_recv error %d\n", __LINE__, len);
        return len;
    }
    rc = signbus_io_send(src,0,data,len);
    if (rc < 0) {
        printf("%d: signbus_io_send error %d\n", __LINE__, rc);
        return rc;
    }

    len = signbus_io_recv(1024,data,&encrypted,&src);
    if (len < 0) {
        printf("%d: signbus_io_recv error %d\n", __LINE__, len);
        return len;
    }
    rc = signbus_io_send(src,0,data,len);
    if (rc < 0) {
        printf("%d: signbus_io_send error %d\n", __LINE__, rc);
        return rc;
    }

    len = signbus_io_recv(1024,data,&encrypted,&src);
    if (len < 0) {
        printf("%d: signbus_io_recv error %d\n", __LINE__, len);
        return len;
    }
    rc = signbus_io_send(src,0,data,len);
    if (rc < 0) {
        printf("%d: signbus_io_send error %d\n", __LINE__, rc);
        return rc;
    }

    len = signbus_io_recv(1024,data,&encrypted,&src);
    if (len < 0) {
        printf("%d: signbus_io_recv error %d\n", __LINE__, len);
        return len;
    }
    rc = signbus_io_send(src,0,data,len);
    if (rc < 0) {
        printf("%d: signbus_io_send error %d\n", __LINE__, rc);
        return rc;
    }
}
