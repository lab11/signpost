#include <stdio.h>

#include <gpio.h>
#include <gpio_async.h>
#include <tock.h>

#include "controller.h"



// Callback when the pressure reading is ready
static void gpio_async_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}


int main(void) {
    putstr("[Controller] Init Backplane\n");


    gpio_async_set_callback(gpio_async_callback, NULL);

    controller_init_module_switches();

    controller_all_modules_enable_power();
    controller_all_modules_enable_i2c();
    controller_all_modules_disable_usb();
}
