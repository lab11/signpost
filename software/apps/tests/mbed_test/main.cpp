#include "mbed.h"
#include <stdio.h>

PortOut Led1(PortB, 0x01);
Serial DBG(SERIAL_TX, SERIAL_RX, 115200);

int main(void) {
    printf("Testing mbed\n");

    while(1) {
        Led1 = 1;
        wait(0.5);
        Led1 = 0;
        wait(0.5);
    }
}
