#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "tock.h"
#include "timer.h"
#include "console.h"
#include "xdot.h"
#include "led.h"

#define LORA_CONSOLE 109

int main (void) {
    printf("Starting Lora Test\n");

    uint8_t AppEUI[8] = {0xc0, 0x98, 0xe5, 0xc0, 0x00, 0x00, 0x00, 0x00};
    uint8_t AppKey[16] = {0};
    AppKey[15] = 0x01;

    xdot_join_network(AppEUI, AppKey);
    delay_ms(1000);
    xdot_set_txdr(4);
    delay_ms(1000);
    xdot_set_ack(1);
    delay_ms(1000);

    while(1) {
        xdot_send((uint8_t*)"Hi From Lab11",13);
        delay_ms(5000);
    }
}
