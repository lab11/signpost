#include <stdio.h>

#include "firestorm.h"
#include "gpio.h"
#include "i2c_master_slave.h"

uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];

// Callback when the pressure reading is ready
static void i2c_master_slave_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}

int main (void) {
    printf("[Ambient Fake]\n");

    // For LED
    gpio_enable_output(8);

    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
    i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

    i2c_master_slave_set_slave_address(0x32);

    while (1) {
        gpio_toggle(8);

        master_write_buf[0] = 0x32;
        master_write_buf[1] = 0x01;
        master_write_buf[2] = 0x00;
        master_write_buf[3] += 1;
        master_write_buf[4] = 0x00;
        master_write_buf[5] += 2;
        master_write_buf[6] = 0x00;
        master_write_buf[7] += 3;
        master_write_buf[8] = 0x00;
        master_write_buf[9] += 4;

        i2c_master_slave_write(0x22, 10);
        yield();

        delay_ms(4000);
    }
}
