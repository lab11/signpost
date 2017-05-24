#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "tock.h"
#include "timer.h"
#include "xdot.h"
#include "multi_console.h"

#define LORA_CONSOLE 109

#define RESPONSE_BUF_SIZE 200

static uint8_t response_buffer[RESPONSE_BUF_SIZE];
static int response_len;

static int check_buffer(uint8_t* buf, int len) {
    //did it end in OK or ERROR?
    if(len >= 5) {
        if(strncmp(buf+len-5,"OK\r\r\n",5)) {
            return 0;
        }
    }

    if(len >= 8) {
        if(strncmp(buf+len-8,"ERROR\r\r\n",8)) { 
            return 1;
        }
    } 

    return -1;
}

static int wait_for_response(void) { 

    //try to wait for the respone once
    int len = console_read(LORA_CONSOLE, response_buffer, RESPONSE_BUF_SIZE);
    
    int check = check_buffer(response_buffer, len);

    if(check == 0 || check == 1) {
        response_len = len;
        return check;
    }

    //we didn't find the response the first time so try again
    response_len = len;
    len = console_read(LORA_CONSOLE, response_buffer+len, RESPONSE_BUF_SIZE-len);
    response_len += len;
    check = check_buffer(response_buffer,response_len);
    
    return check;
}


int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey) {
    const char* cmd = "AT\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(500);

    cmd = "AT+NJM=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(500);

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
    delay_ms(500);

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
    delay_ms(500);

    cmd = "AT+PN=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(500);

    cmd = "AT+FSB=1\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(500);

    cmd = "AT&W\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(500);

    cmd = "ATZ\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(3000);

    cmd = "AT+JOIN\n";
    console_write(LORA_CONSOLE, (uint8_t*)cmd, strlen(cmd));
    delay_ms(5000);

    return 0;
}

int xdot_get_txdr(void) {
    char cmd[15];
    sprintf(cmd, "AT+TXDR?\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    int ret = wait_for_response();

    if(ret == -1) {
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
                return -1;
            } else {
                return a;
            }
        }
    } else {
        return -1;
    }
}

int xdot_set_txdr(uint8_t dr) {

    if(dr > 4) {
        return -1;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXDR=%d\n", dr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));

    return 0;
}

int xdot_set_adr(uint8_t adr) {

    if(adr > 1) {
        return -1;
    }

    char cmd[15];
    sprintf(cmd, "AT+ADR=%d\n", adr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));

    return 0;
}

int xdot_set_txpwr(uint8_t tx) {

    if(tx > 20) {
        return -1;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXP=%d\n", tx);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return wait_for_response();
}

int xdot_set_ack(uint8_t ack) {

    if(ack != 0 && ack != 1) {
        return -1;
    }

    char cmd[15];
    sprintf(cmd, "AT+ACK=%d\n",ack);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));

    return 0;
}

int xdot_send(uint8_t* buf, uint8_t len) {
    const char* cmd = "AT+send=";
    console_write(LORA_CONSOLE, (uint8_t*)cmd , strlen(cmd));
    console_write(LORA_CONSOLE, buf, len);
    console_write(LORA_CONSOLE, (uint8_t*)"\n", 1);

    return 0;
}
