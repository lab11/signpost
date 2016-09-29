#include <firestorm.h>
#include <gpio.h>
#include <gpio_async.h>
#include <stdio.h>

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


int main(void) {
    putstr("Backplane Test222\n");

    gpio_enable_output(LED_0);

    gpio_async_set_callback(gpio_async_callback, NULL);

    gpio_async_enable_output(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_POWER);
    yield();

    gpio_async_enable_output(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_I2C);
    yield();

    gpio_async_enable_output(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_USB);
    yield();


    while(1) {
        gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_POWER);
        yield();

        gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_I2C);
        yield();

        gpio_async_set(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_USB);
        yield();

        gpio_toggle(LED_0);
        delay_ms(500);

        gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_POWER);
        yield();

        gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_I2C);
        yield();

        gpio_async_clear(MOD7_GPIO_ASYNC_PORT_NUM, PIN_IDX_ISOLATE_USB);
        yield();

        gpio_toggle(LED_0);
        delay_ms(500);
    }
}

