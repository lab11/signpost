#include <stdint.h>
#include <stdio.h>
#include "sara_u260.h"
#include "tock.h"
#include <stdlib.h>
#include "at_command.h"

#define SARA_CONSOLE 110

int sara_u260_init(void) {
    at_send(SARA_CONSOLE,"AT\r");

    for(volatile uint32_t i = 0; i < 15000; i++);

    at_send(SARA_CONSOLE,"ATE0\r");
    int ret = at_wait_for_response(SARA_CONSOLE,3);
    if(ret >= AT_SUCCESS) {
        return SARA_U260_SUCCESS;
    }
}

static int sara_u260_check_connection(void) {
    at_send(SARA_CONSOLE, "AT+COPS?\r");

    uint8_t buf[50];
    int len = at_get_response(SARA_CONSOLE, 3, buf, 50);

    if(len <= 0) {
        return SARA_U260_ERROR;
    }else if(len > 20) {
        return SARA_U260_SUCCESS;
    } else {
        return SARA_U260_NO_SERVICE;
    }
}

static int sara_u260_setup_packet_switch(void) {
    //check the connection
    int ret = sara_u260_check_connection();
    if(ret != SARA_U260_SUCCESS) {
        return ret;
    }

    //do we already have a pack switch setup?
    at_send(SARA_CONSOLE,"AT+UPSND=0,0\r");
    ret = at_wait_for_response(SARA_CONSOLE, 3);
    if(ret == AT_ERROR) {
        //no we need to set one up

        //set the apn for our network - it can be blank
        at_send(SARA_CONSOLE, "AT+UPSD=0,1,\"\"\r");
        at_wait_for_response(SARA_CONSOLE, 3);

        //request to connect
        at_send(SARA_CONSOLE, "AT+UPSDA=0,3\r");
        at_wait_for_response(SARA_CONSOLE, 3);

        //did it work
        at_send(SARA_CONSOLE,"AT+UPSND=0,0\r");
        ret = at_wait_for_response(SARA_CONSOLE, 3);

        if(ret < 0) {
            //no it didn't - return error
            return SARA_U260_ERROR;
        }

    } else if (ret == AT_NO_RESPONSE) {
        //nothign to do about this - return an error
        return SARA_U260_ERROR;
    }

    return SARA_U260_SUCCESS;
}

static int sara_u260_del_file(const char* fname) {
    char c[50];
    int clen = sprintf(c, "AT+UDELFILE=\"%s\"\r", fname);
    at_send_buf(SARA_CONSOLE, c, clen);
    int ret = at_wait_for_response(SARA_CONSOLE,3);

    if(ret >= 0) {
        return SARA_U260_SUCCESS;
    } else {
        return SARA_U260_ERROR;
    }
}

static int sara_u260_write_to_file(const char* fname, uint8_t* buf, size_t len) {

    char c[50];
    int clen = sprintf(c, "AT+UDWNFILE=\"%s\",%d\r", fname, len);
    at_send_buf(SARA_CONSOLE, c, clen);
    at_wait_for_custom_response(SARA_CONSOLE,3,"\n>");

    //now send the buffer in chunks of 30
    for(size_t i = 0; i < len; i+=30) {
        if(i+30 <= len) {
            at_send_buf(SARA_CONSOLE, buf+i, 30);
        } else {
            at_send_buf(SARA_CONSOLE, buf+i, len-i);
        }
    }

    int ret = at_wait_for_response(SARA_CONSOLE,3);
    if(ret >= AT_SUCCESS) {
        return SARA_U260_SUCCESS;
    } else {
        return SARA_U260_ERROR;
    }
}

int sara_u260_basic_http_post(const char* url, const char* path, uint8_t* buf, size_t len) {

    //make the connection
    int ret = sara_u260_setup_packet_switch();
    if(ret < 0) {
        return ret;
    }

    //delete the file
    ret = sara_u260_del_file("postdata.bin");
    if(ret < 0) {
        return ret;
    }

    //write the data to a file
    ret = sara_u260_write_to_file("postdata.bin", buf, len);
    if(ret < 0) {
        return ret;
    }

    //setup http profile
    at_send(SARA_CONSOLE,"AT+UHTTP=0\r");
    at_wait_for_response(SARA_CONSOLE, 3);

    at_send(SARA_CONSOLE,"AT+UHTTP=0,1,\"");
    at_send(SARA_CONSOLE,url);
    at_send(SARA_CONSOLE,"\"\r");
    at_wait_for_response(SARA_CONSOLE, 3);

    at_send(SARA_CONSOLE,"AT+UHTTP=0,5,80\r");
    at_wait_for_response(SARA_CONSOLE, 3);

    //now actually do the post
    at_send(SARA_CONSOLE,"AT+UHTTPC=0,4,\"");
    at_send(SARA_CONSOLE,path);
    at_send(SARA_CONSOLE,"\",\"postresult.txt\",\"postdata.bin\",2\r");
    ret = at_wait_for_response(SARA_CONSOLE, 3);

    return ret;
}

int sara_u260_authenticated_http_post(const char* url, const char* path, uint8_t* buf,
                                        size_t len, const char* username, const char* password) {

}

int sara_u260_get_post_response(uint8_t* buf, size_t max_len) {

}
