#include <firestorm.h>
#include <gpio.h>
#include <gpio_async.h>
#include <stdio.h>


#include "i2c_master_slave.h"

// #define MOD0_GPIO_ASYNC_PORT_NUM 0
// #define MOD1_GPIO_ASYNC_PORT_NUM 1
// #define MOD2_GPIO_ASYNC_PORT_NUM 2
// #define MOD5_GPIO_ASYNC_PORT_NUM 3
// #define MOD6_GPIO_ASYNC_PORT_NUM 4
// #define MOD7_GPIO_ASYNC_PORT_NUM 5

// #define PIN_IDX_ISOLATE_POWER   0
// #define PIN_IDX_ISOLATE_I2C     1
// #define PIN_IDX_ISOLATE_USB     2

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

    // putstr("callback ");
    // unsigned i;
    // for (i=0; i<count; i++) {
    //     putstr("X");
    // }
    // putstr("C\n");

    // count += addend;
    // if (count > 10) addend = -1;
    // if (count == 0) addend = 1;
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

    // putstr("callback ");
    // unsigned i;
    // for (i=0; i<count; i++) {
    //     putstr("X");
    // }
    // putstr("C\n");

    // count += addend;
    // if (count > 10) addend = -1;
    // if (count == 0) addend = 1;
    // printf("i2c\n");
}




int main(void) {
    printf("Test I2C\n");

    gpio_enable_output(8);

    gpio_toggle(8);
    // delay_ms(500);
    // gpio_toggle(8);
    // delay_ms(500);
    // gpio_toggle(8);
    // delay_ms(500);

    // // yield();

    // // putstr("WHEHRE\n");

    // gpio_async_set_callback(gpio_async_callback, NULL);


    // configure_module(MOD7_GPIO_ASYNC_PORT_NUM);
    // configure_module(MOD6_GPIO_ASYNC_PORT_NUM);
    // configure_module(MOD5_GPIO_ASYNC_PORT_NUM);
    // configure_module(MOD2_GPIO_ASYNC_PORT_NUM);
    // configure_module(MOD1_GPIO_ASYNC_PORT_NUM);
    // configure_module(MOD0_GPIO_ASYNC_PORT_NUM);

    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
    i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

    i2c_master_slave_set_slave_address(0x07);

    i2c_master_slave_listen();
    // yield();








    while(1) {

        master_write_buf[0] = 0x8;
        master_write_buf[1] = 0xb6;
        master_write_buf[2] = 0x91;
        master_write_buf[3] = 0x55;
        master_write_buf[4] = 0x56;
        master_write_buf[5] = 0x57;
        master_write_buf[6] = 0x58;
        master_write_buf[7] = 0x59;

        i2c_master_slave_write(0x20, 8);



        // gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_POWER);
        yield();

        gpio_toggle(8);
        // delay_ms(200);
        // gpio_toggle(8);
        // delay_ms(200);
        // gpio_toggle(8);
        // delay_ms(200);

        // printf("got 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", slave_write_buf[0], slave_write_buf[1], slave_write_buf[2], slave_write_buf[3], slave_write_buf[4]);

        // i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

        // gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_I2C);
        // yield();

        // gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_USB);
        // yield();

        // gpio_toggle(LED_0);
        // // yield();
        // delay_ms(500);

        // gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_POWER);
        // yield();

        // gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_I2C);
        // yield();

        // gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_USB);
        // yield();

        // gpio_toggle(LED_0);
        // // yield();
        delay_ms(500);
    }
}

