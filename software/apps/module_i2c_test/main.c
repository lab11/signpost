#include <stdio.h>

#include "tock.h"
#include "console.h"
#include "gpio.h"
#include "i2c_master_slave.h"

uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];

// Callback when the pressure reading is ready
static void gpio_async_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
    static unsigned count = 0;
    static int addend = 1;
}

// Callback when the pressure reading is ready
static void i2c_master_slave_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
    static unsigned count = 0;
    static int addend = 1;
}

int main(void) {
    printf("Test I2C\n");

    gpio_enable_output(8);
    gpio_toggle(8);


    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
    i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

    i2c_master_slave_set_slave_address(0x07);

    i2c_master_slave_listen();


    while(1) {
        master_write_buf[0] = 0x8;
        master_write_buf[1] = 0xb6;
        master_write_buf[2] = 0x91;
        master_write_buf[3] = 0x55;
        master_write_buf[4] = 0x56;
        master_write_buf[5] = 0x57;
        master_write_buf[6] = 0x58;
        master_write_buf[7] = 0x59;

        i2c_master_slave_write_sync(0x20, 8);

        gpio_toggle(8);
        delay_ms(500);
    }
}
