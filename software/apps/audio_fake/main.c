#include <stdio.h>

#include "gpio.h"
#include "timer.h"
#include "i2c_master_slave.h"

uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256] = {0};

int main (void) {
    printf("[Audio Fake]\n");

    // For LED
    gpio_enable_output(10);

    i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
    i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

    i2c_master_slave_set_slave_address(0x33);

    while (1) {
        gpio_toggle(10);

        master_write_buf[0] = 0x33;
        master_write_buf[1] = 0;
        master_write_buf[2] += 1; // first band
        master_write_buf[3] += 2;
        master_write_buf[4] += 1; // band 2
        master_write_buf[5] += 10;
        master_write_buf[6] += 1; // band 3
        master_write_buf[7] += 88;
        master_write_buf[8] += 1; // band 4
        master_write_buf[9] += 145;
        master_write_buf[10] += 1; // band 5
        master_write_buf[11] += 17;
        master_write_buf[12] += 1; // band 6
        master_write_buf[13] += 5;
        master_write_buf[14] += 1; // band 7
        master_write_buf[15] += 200;

        i2c_master_slave_write_sync(0x22, 16);

        delay_ms(10000);
    }
}
