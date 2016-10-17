#include <stdio.h>

#include "gpio.h"
#include "timer.h"
#include "i2c_master_slave.h"

uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];


int main (void) {
    printf("[Microwave Radar Fake]\n");

    // For LED
    gpio_enable_output(9);

    i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
    i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

    i2c_master_slave_set_slave_address(0x34);

    while (1) {
        gpio_toggle(9);

        master_write_buf[0] = 0x34;
        master_write_buf[1] = 0x00;
        master_write_buf[2] = 0x00;
        master_write_buf[3] = 0x00;
        master_write_buf[4] = 0x00;
        master_write_buf[5] += 2;
        master_write_buf[6] = 0x00;
        master_write_buf[7] = 0x00;
        master_write_buf[8] = 0x00;
        master_write_buf[9] += 3;

        i2c_master_slave_write_sync(0x22, 10);

        delay_ms(3000);
    }
}
