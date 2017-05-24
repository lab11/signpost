#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tock.h"
#include "timer.h"
#include "xdot.h"
#include "multi_console.h"

#define LORA_CONSOLE 109

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

int xdot_set_txdr(uint8_t dr) {

    if(dr > 4) {
        return -1;
    }

    char cmd[15];
    sprintf(cmd, "AT+TXDR=%d\n", dr);
    console_write(LORA_CONSOLE, cmd, strlen(cmd));

    return 0;
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
