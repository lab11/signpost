#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "at_command.h"
#include "multi_console.h"


static int check_buffer(uint8_t* buf, int len) {
    //did it end in OK or ERROR?
    if(len >= 4) {
        if(!strncmp(buf+len-4,"OK\r\n",4)) {
            return AT_SUCCESS;
        }
    }

    if(len >= 7) {
        if(!strncmp(buf+len-7,"ERROR\r\n",7)) {
            return AT_ERROR;
        }
    }

    return AT_NO_RESPONSE;
}

static int check_custom_buffer(uint8_t* buf, int len, const char* rstring) {

    int rlen = strlen(rstring);
    if(len >= rlen) {
        if(!strncmp(buf+len-rlen,rstring,rlen)) {
            return AT_SUCCESS;
        }
    }

    return AT_NO_RESPONSE;
}

int at_send(int console_num, const char* cmd) {
    console_write(console_num, (uint8_t*) cmd, strlen(cmd));
    return AT_SUCCESS;
}

int at_send_buf(int console_num, uint8_t* buf, size_t len) {
    console_write(console_num, buf, len);
    return AT_SUCCESS;
}


int at_wait_for_response(int console_num, uint8_t max_tries) {
    static uint8_t buf[200];
    return at_get_response(console_num, max_tries, buf, 200);
}

int at_wait_for_custom_response(int console_num, uint8_t max_tries, const char* rstring) {
    static uint8_t buf[200];
    return at_get_custom_response(console_num, max_tries, buf, 200, rstring);
}

int at_get_response(int console_num, uint8_t max_tries, uint8_t* buf, size_t max_len) {

    int tlen = 0;
    for(uint8_t i = 0; i < max_tries; i++) {

        int len = console_read(console_num, buf+tlen, max_len-tlen);
        tlen += len;
        int check = check_buffer(buf, tlen);

        if(check == AT_SUCCESS) {
            return tlen;
        } else if (check == AT_ERROR) {
            return AT_ERROR;
        }
    }

    return AT_NO_RESPONSE;
}

int at_get_custom_response(int console_num, uint8_t max_tries, uint8_t* buf, size_t max_len, const char* rstring) {

    int tlen = 0;
    for(uint8_t i = 0; i < max_tries; i++) {

        int len = console_read(console_num, buf+tlen, max_len-tlen);
        printf("Got len: %d\n", len);
        tlen += len;
        int check = check_custom_buffer(buf, tlen, rstring);

        if(check == AT_SUCCESS) {
            return tlen;
        } else if (check == AT_ERROR) {
            return AT_ERROR;
        }
    }

    return AT_NO_RESPONSE;
}
