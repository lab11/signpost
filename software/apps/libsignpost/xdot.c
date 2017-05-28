#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "tock.h"
#include "console.h"
#include "led.h"
#include "gpio.h"
#include "timer.h"
#include "xdot.h"
#include "multi_console.h"
#include "at_command.h"

#define LORA_CONSOLE 109

#define LORA_WAKE_PIN 10

#define RESPONSE_BUF_SIZE 200



int xdot_init(void) {
    gpio_enable_output(LORA_WAKE_PIN);

    at_send(LORA_CONSOLE, "ATE0\n");
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey) {
    at_send(LORA_CONSOLE, "AT\n");
    at_wait_for_response(LORA_CONSOLE,3);

    at_send(LORA_CONSOLE, "AT+NJM=1\n");
    at_wait_for_response(LORA_CONSOLE,3);

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
    at_send_buf(LORA_CONSOLE,c,len);
    at_wait_for_response(LORA_CONSOLE,3);

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
    at_send_buf(LORA_CONSOLE,c,len);
    at_wait_for_response(LORA_CONSOLE,3);

    at_send(LORA_CONSOLE,"AT+PN=1\n");
    at_wait_for_response(LORA_CONSOLE,3);

    at_send(LORA_CONSOLE,"AT+FSB=1\n");
    at_wait_for_response(LORA_CONSOLE,3);

    at_send(LORA_CONSOLE,"AT&W\n");
    at_wait_for_response(LORA_CONSOLE,3);

    xdot_reset();

    at_send(LORA_CONSOLE,"AT+JOIN\n");
    at_wait_for_response(LORA_CONSOLE,3);

    return XDOT_SUCCESS;
}

int xdot_get_txdr(void) {
    char cmd[15];
    sprintf(cmd, "AT+TXDR?\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));

    uint8_t buf[200];
    int ret = at_get_response(LORA_CONSOLE, 3, buf, 200);

    if(ret <= 0) {
        return ret;
    }

    if(buf[0] == '0') {
        return 0;
    } else {
        char c[2];
        snprintf(c,1,"%s",(char*)buf);
        int a = atoi(c);
        if(a == 0) {
            return XDOT_ERROR;
        } else {
            return a;
        }
    }
}

int xdot_set_txdr(uint8_t dr) {

    if(dr > 4) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXDR=%d\n", dr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_set_adr(uint8_t adr) {

    if(adr > 1) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+ADR=%d\n", adr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_set_txpwr(uint8_t tx) {

    if(tx > 20) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXP=%d\n", tx);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_set_ack(uint8_t ack) {

    if(ack > 8) {
        return XDOT_INVALID_PARAM;
    }

    char cmd[15];
    sprintf(cmd, "AT+ACK=%d\n",ack);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_save_settings(void) {
    char cmd[15];
    sprintf(cmd, "AT&W\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_reset(void) {
    char cmd[15];
    sprintf(cmd, "ATZ\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    at_wait_for_response(LORA_CONSOLE,3);

    for(volatile uint32_t i = 0; i < 1500000; i++);

    sprintf(cmd, "AT\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);

}

int xdot_send(uint8_t* buf, uint8_t len) {
    const char* cmd = "AT+send=";
    console_write(LORA_CONSOLE, (uint8_t*)cmd , strlen(cmd));
    console_write(LORA_CONSOLE, buf, len);
    console_write(LORA_CONSOLE, (uint8_t*)"\n", 1);
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_sleep(void) {
    char cmd[15];
    sprintf(cmd, "AT+WM=1\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    at_wait_for_response(LORA_CONSOLE,3);

    sprintf(cmd, "AT+sleep\n");
    console_write(LORA_CONSOLE, cmd, strlen(cmd));
    return at_wait_for_response(LORA_CONSOLE,3);
}

int xdot_wake(void) {
    gpio_toggle(LORA_WAKE_PIN);
    gpio_toggle(LORA_WAKE_PIN);

    for(volatile uint32_t i = 0; i < 15000; i++);

    return XDOT_SUCCESS;
}
