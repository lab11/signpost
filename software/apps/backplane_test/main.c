#include <firestorm.h>
#include <gpio.h>
#include <gpio_async.h>
#include <stdio.h>

#include "controller.h"

#define MOD0_GPIO_ASYNC_PORT_NUM 0
#define MOD1_GPIO_ASYNC_PORT_NUM 1
#define MOD2_GPIO_ASYNC_PORT_NUM 2
#define MOD5_GPIO_ASYNC_PORT_NUM 3
#define MOD6_GPIO_ASYNC_PORT_NUM 4
#define MOD7_GPIO_ASYNC_PORT_NUM 5

#define PIN_IDX_ISOLATE_POWER   0
#define PIN_IDX_ISOLATE_I2C     1
#define PIN_IDX_ISOLATE_USB     2

// Callback when the pressure reading is ready
static void gpio_async_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
    static unsigned count = 0;
    static int addend = 1;

    putstr("callback ");
    unsigned i;
    for (i=0; i<count; i++) {
        putstr("X");
    }
    putstr("\n");

    count += addend;
    if (count > 10) addend = -1;
    if (count == 0) addend = 1;
}


void test_module(uint32_t gpio_async_port_number) {
        /////////////////////////////
        gpio_async_set(gpio_async_port_number, PIN_IDX_ISOLATE_POWER);
        yield();

        gpio_async_set(gpio_async_port_number, PIN_IDX_ISOLATE_I2C);
        yield();

        gpio_async_set(gpio_async_port_number, PIN_IDX_ISOLATE_USB);
        yield();

        gpio_toggle(LED_0);
        delay_ms(500);

        gpio_async_clear(gpio_async_port_number, PIN_IDX_ISOLATE_POWER);
        yield();

        gpio_async_clear(gpio_async_port_number, PIN_IDX_ISOLATE_I2C);
        yield();

        gpio_async_clear(gpio_async_port_number, PIN_IDX_ISOLATE_USB);
        yield();

        gpio_toggle(LED_0);
        delay_ms(500);
}


int main(void) {
    putstr("Backplane Test\n");

    gpio_enable_output(LED_0);

    gpio_async_set_callback(gpio_async_callback, NULL);

    controller_init_module_switches();

    putstr("Test Module 0\n");
    test_module(MOD0_GPIO_ASYNC_PORT_NUM);
    putstr("Test Module 1\n");
    test_module(MOD1_GPIO_ASYNC_PORT_NUM);
    putstr("Test Module 2\n");
    test_module(MOD2_GPIO_ASYNC_PORT_NUM);
    putstr("Test Module 5\n");
    test_module(MOD5_GPIO_ASYNC_PORT_NUM);
    putstr("Test Module 6\n");
    test_module(MOD6_GPIO_ASYNC_PORT_NUM);
    putstr("Test Module 7\n");
    test_module(MOD7_GPIO_ASYNC_PORT_NUM);

    putstr("Enable all modules\n");
    controller_all_modules_enable_power();
    controller_all_modules_enable_i2c();
    controller_all_modules_enable_usb();

    putstr("Backplane Test Complete.\n");
}

