#include "gpio_async.h"
#include "controller.h"


static uint8_t module_to_async_port_num[8] = {0, 1, 2, 0xff, 0xff, 3, 4, 5};

// Configure all pins of the GPIO extenders on the backplane to be outputs
void controller_init_module_switches () {
    int i;

    for (i=0; i<6; i++) {
        gpio_async_enable_output(i, PIN_IDX_ISOLATE_POWER);
        yield();
        gpio_async_enable_output(i, PIN_IDX_ISOLATE_I2C);
        yield();
        gpio_async_enable_output(i, PIN_IDX_ISOLATE_USB);
        yield();
    }
}

// Supply power to a given module
void controller_module_enable_power (module_num_t module_number) {
    gpio_async_set(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_POWER);
    yield();
}

// Enable a module's I2C
void controller_module_enable_i2c (module_num_t module_number) {
    gpio_async_set(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_I2C);
    yield();
}

// Enable a module's USB
void controller_module_enable_usb (module_num_t module_number) {
    gpio_async_set(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_USB);
    yield();
}

void controller_all_modules_enable_power () {
    controller_module_enable_power(MODULE0);
    controller_module_enable_power(MODULE1);
    controller_module_enable_power(MODULE2);
    controller_module_enable_power(MODULE5);
    controller_module_enable_power(MODULE6);
    controller_module_enable_power(MODULE7);
}

void controller_all_modules_enable_i2c () {
    controller_module_enable_i2c(MODULE0);
    controller_module_enable_i2c(MODULE1);
    controller_module_enable_i2c(MODULE2);
    controller_module_enable_i2c(MODULE5);
    controller_module_enable_i2c(MODULE6);
    controller_module_enable_i2c(MODULE7);
}

void controller_all_modules_enable_usb () {
    controller_module_enable_usb(MODULE0);
    controller_module_enable_usb(MODULE1);
    controller_module_enable_usb(MODULE2);
    controller_module_enable_usb(MODULE5);
    controller_module_enable_usb(MODULE6);
    controller_module_enable_usb(MODULE7);
}

// Shut off power to a given module
void controller_module_disable_power (module_num_t module_number) {
    gpio_async_clear(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_POWER);
    yield();
}

// Disable a module's I2C
void controller_module_disable_i2c (module_num_t module_number) {
    gpio_async_clear(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_I2C);
    yield();
}

// Disable a module's USB
void controller_module_disable_usb (module_num_t module_number) {
    gpio_async_clear(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_USB);
    yield();
}

void controller_all_modules_disable_power () {
    controller_module_disable_power(MODULE0);
    controller_module_disable_power(MODULE1);
    controller_module_disable_power(MODULE2);
    controller_module_disable_power(MODULE5);
    controller_module_disable_power(MODULE6);
    controller_module_disable_power(MODULE7);
}

void controller_all_modules_disable_i2c () {
    controller_module_disable_i2c(MODULE0);
    controller_module_disable_i2c(MODULE1);
    controller_module_disable_i2c(MODULE2);
    controller_module_disable_i2c(MODULE5);
    controller_module_disable_i2c(MODULE6);
    controller_module_disable_i2c(MODULE7);
}

void controller_all_modules_disable_usb () {
    controller_module_disable_usb(MODULE0);
    controller_module_disable_usb(MODULE1);
    controller_module_disable_usb(MODULE2);
    controller_module_disable_usb(MODULE5);
    controller_module_disable_usb(MODULE6);
    controller_module_disable_usb(MODULE7);
}
