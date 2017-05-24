#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "tock.h"
#include "console.h"
#include "led.h"
#include "timer.h"
#include "xdot.h"
#include "multi_console.h"

#define LORA_CONSOLE 109

#define RESPONSE_BUF_SIZE 200

static uint8_t response_buffer[RESPONSE_BUF_SIZE];
static int response_len;


static int check_buffer(uint8_t* buf, int len) {
    //did it end in OK or ERROR?
    if(len >= 4) {
        if(!strncmp(buf+len-4,"OK\r\n",4)) {
            return XDOT_SUCCESS;
        }
    }

    if(len >= 7) {
        if(!strncmp(buf+len-7,"ERROR\r\n",7)) {
            return XDOT_ERROR;
        }
    }

    return XDOT_NO_RESPONSE;
}

//this really should have a timeout on it - we are bound to get stuck
static int wait_for_response(void) {

    //try to wait for the respone once
    int len = console_read(LORA_CONSOLE, response_buffer, RESPONSE_BUF_SIZE);
    response_len = len;
    int check = check_buffer(response_buffer, len);
    if(check == XDOT_SUCCESS || check == XDOT_ERROR) {
        return check;
    }

    //we didn't find the response the first time so try again
    len = console_read(LORA_CONSOLE, response_buffer+len, RESPONSE_BUF_SIZE-len);
    response_len += len;
    check = check_buffer(response_buffer,response_len);
    if(check == XDOT_SUCCESS || check == XDOT_ERROR) {
        return check;
    }

    //we didn't find the response the seond time so try again
    len = console_read(LORA_CONSOLE, response_buffer+len, RESPONSE_BUF_SIZE-len);
    response_len += len;
    check = check_buffer(response_buffer,response_len);

    return check;
}

int xdot_init(void) {
    const char* cmd = "ATE0\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey) {
    const char* cmd = "AT\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    cmd = "AT+NJM=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    static char c[200];

    uint8_t len = 0;
    strcpy(c, "AT+NI=0,");
    len = 8;
    char* cpt = c+8;
    for(uint8_t i = 0; i < 8; i++) {
        sprintf(cpt,"%02X",AppEUI[i]);
        cpt += 2;
        len += 2;
    }
    sprintf(cpt, "\n");
    len += 1;
    console_write(LORA_CONSOLE, c, len);
    wait_for_response();

    len = 0;
    strcpy(c, "AT+NK=0,");
    len = 8;
    cpt = c+8;
    for(uint8_t i = 0; i < 16; i++) {
        sprintf(cpt,"%02X",AppKey[i]);
        cpt += 2;
        len += 2;
    }
    sprintf(cpt, "\n");
    len += 1;
    console_write(LORA_CONSOLE, c, len);
    wait_for_response();

    cmd = "AT+PN=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    cmd = "AT+FSB=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    cmd = "AT&W\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    xdot_reset();

    cmd = "AT+JOIN\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    wait_for_response();

    return XDOT_SUCCESS;
}

int xdot_get_txdr(void) {
    char cmd[15];
    sprintf(cmd, "AT+TXDR?\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    int ret = wait_for_response();

    if(ret == XDOT_NO_RESPONSE || ret == XDOT_ERROR) {
        return ret;
    }

    if(response_len >= 1) {
        if(response_buffer[0] == '0') {
            return 0;
        } else {
            char c[2];
            snprintf(c,1,"%s",(char*)response_buffer);
            int a = atoi(c);
            if(a == 0) {
                return XDOT_ERROR;
            } else {
                return a;
            }
        }
    } else {
        return XDOT_NO_RESPONSE;
    }
}

int xdot_set_txdr(uint8_t dr) {

    if(dr > 4) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXDR=%d\n", dr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_set_adr(uint8_t adr) {

    if(adr > 1) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+ADR=%d\n", adr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_set_txpwr(uint8_t tx) {

    if(tx > 20) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXP=%d\n", tx);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_set_ack(uint8_t ack) {

    if(ack != 0 && ack != 1) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+ACK=%d\n",ack);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_save_settings(void) {
    char cmd[15];
    sprintf(cmd, "AT&W\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_reset(void) {
    char cmd[15];
    sprintf(cmd, "ATZ\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    wait_for_response();

    for(volatile uint32_t i = 0; i < 1500000; i++);

    sprintf(cmd, "AT\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();

}

int xdot_send(uint8_t* buf, uint8_t len) {
    const char* cmd = "AT+send=";
    console_write(LORA_CONSOLE, (uint8_t*)cmd , strlen(cmd));
    console_write(LORA_CONSOLE, buf, len);
    console_write(LORA_CONSOLE, (uint8_t*)"\n", 1);
    return wait_for_response();
}
