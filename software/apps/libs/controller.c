#include "gpio.h"
#include "gpio_async.h"
#include "controller.h"

const uint8_t MOD_OUTS[NUM_MOD_IO] =  {MOD0_OUT,
                                       MOD1_OUT,
                                       MOD2_OUT,
                                       MOD5_OUT,
                                       MOD6_OUT,
                                       MOD7_OUT};

const uint8_t MOD_INS[NUM_MOD_IO] =   {MOD0_IN,
                                       MOD1_IN,
                                       MOD2_IN,
                                       MOD5_IN,
                                       MOD6_IN,
                                       MOD7_IN};

static uint8_t module_to_async_port_num[8] = {0, 1, 2, 0xff, 0xff, 3, 4, 5};

// Configure all pins of the GPIO extenders on the backplane to be outputs
void controller_init_module_switches (void) {
    int i;

    for (i=0; i<6; i++) {
        gpio_async_enable_output_sync(i, PIN_IDX_ISOLATE_POWER);
        gpio_async_enable_output_sync(i, PIN_IDX_ISOLATE_I2C);
        gpio_async_enable_output_sync(i, PIN_IDX_ISOLATE_USB);
    }
}

void controller_gpio_enable_all_MODINs (void) {
    gpio_enable_output(MOD0_IN);
    gpio_enable_output(MOD1_IN);
    gpio_enable_output(MOD2_IN);
    gpio_enable_output(MOD5_IN);
    gpio_enable_output(MOD6_IN);
    gpio_enable_output(MOD7_IN);
}

void controller_gpio_enable_all_MODOUTs (GPIO_InputMode_t pin_config) {
    gpio_enable_input(MOD0_OUT, pin_config);
    gpio_enable_input(MOD1_OUT, pin_config);
    gpio_enable_input(MOD2_OUT, pin_config);
    gpio_enable_input(MOD5_OUT, pin_config);
    gpio_enable_input(MOD6_OUT, pin_config);
    gpio_enable_input(MOD7_OUT, pin_config);
}

module_num_t MODOUT_pin_to_mod_name (GPIO_Pin_enum pin) {
    switch(pin) {
        case MOD0_OUT:
            return MODULE0;
        case MOD1_OUT:
            return MODULE1;
        case MOD2_OUT:
            return MODULE2;
        case MOD5_OUT:
            return MODULE5;
        case MOD6_OUT:
            return MODULE6;
        case MOD7_OUT:
            return MODULE7;
        default:
            break;
    }
    return 0xff;
}

module_num_t MODIN_pin_to_mod_name (GPIO_Pin_enum pin) {
    switch(pin) {
        case MOD0_IN:
            return MODULE0;
        case MOD1_IN:
            return MODULE1;
        case MOD2_IN:
            return MODULE2;
        case MOD5_IN:
            return MODULE5;
        case MOD6_IN:
            return MODULE6;
        case MOD7_IN:
            return MODULE7;
        default:
            break;
    }
    return 0xff;
}
GPIO_Pin_enum mod_name_to_MODOUT_pin (module_num_t module) {
    switch(module) {
        case MODULE0:
            return MOD0_OUT;
        case MODULE1:
            return MOD1_OUT;
        case MODULE2:
            return MOD2_OUT;
        case MODULE5:
            return MOD5_OUT;
        case MODULE6:
            return MOD6_OUT;
        case MODULE7:
            return MOD7_OUT;
        default:
            break;
    }
    return 0xff;
}

GPIO_Pin_enum mod_name_to_MODIN_pin (module_num_t module) {
    switch(module) {
        case MODULE0:
            return MOD0_IN;
        case MODULE1:
            return MOD1_IN;
        case MODULE2:
            return MOD2_IN;
        case MODULE5:
            return MOD5_IN;
        case MODULE6:
            return MOD6_IN;
        case MODULE7:
            return MOD7_IN;
        default:
            break;
    }
    return 0xff;
}

void controller_gpio_set_all (void) {
    gpio_set(MOD0_IN);
    gpio_set(MOD1_IN);
    gpio_set(MOD2_IN);
    gpio_set(MOD5_IN);
    gpio_set(MOD6_IN);
    gpio_set(MOD7_IN);
}

void controller_gpio_clear_all (void) {
    gpio_clear(MOD0_IN);
    gpio_clear(MOD1_IN);
    gpio_clear(MOD2_IN);
    gpio_clear(MOD5_IN);
    gpio_clear(MOD6_IN);
    gpio_clear(MOD7_IN);
}

// Supply power to a given module
void controller_module_enable_power (module_num_t module_number) {
    gpio_async_set_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_POWER);
}

// Enable a module's I2C
void controller_module_enable_i2c (module_num_t module_number) {
    gpio_async_set_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_I2C);
}

// Enable a module's USB
void controller_module_enable_usb (module_num_t module_number) {
    gpio_async_set_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_USB);
}

void controller_all_modules_enable_power (void) {
    controller_module_enable_power(MODULE0);
    controller_module_enable_power(MODULE1);
    controller_module_enable_power(MODULE2);
    controller_module_enable_power(MODULE5);
    controller_module_enable_power(MODULE6);
    controller_module_enable_power(MODULE7);
}

void controller_all_modules_enable_i2c (void) {
    controller_module_enable_i2c(MODULE0);
    controller_module_enable_i2c(MODULE1);
    controller_module_enable_i2c(MODULE2);
    controller_module_enable_i2c(MODULE5);
    controller_module_enable_i2c(MODULE6);
    controller_module_enable_i2c(MODULE7);
}

void controller_all_modules_enable_usb (void) {
    controller_module_enable_usb(MODULE0);
    controller_module_enable_usb(MODULE1);
    controller_module_enable_usb(MODULE2);
    controller_module_enable_usb(MODULE5);
    controller_module_enable_usb(MODULE6);
    controller_module_enable_usb(MODULE7);
}

// Shut off power to a given module
void controller_module_disable_power (module_num_t module_number) {
    gpio_async_clear_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_POWER);
}

// Disable a module's I2C
void controller_module_disable_i2c (module_num_t module_number) {
    gpio_async_clear_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_I2C);
}

// Disable a module's USB
void controller_module_disable_usb (module_num_t module_number) {
    gpio_async_clear_sync(module_to_async_port_num[module_number], PIN_IDX_ISOLATE_USB);
}

void controller_all_modules_disable_power (void) {
    controller_module_disable_power(MODULE0);
    controller_module_disable_power(MODULE1);
    controller_module_disable_power(MODULE2);
    controller_module_disable_power(MODULE5);
    controller_module_disable_power(MODULE6);
    controller_module_disable_power(MODULE7);
}

void controller_all_modules_disable_i2c (void) {
    controller_module_disable_i2c(MODULE0);
    controller_module_disable_i2c(MODULE1);
    controller_module_disable_i2c(MODULE2);
    controller_module_disable_i2c(MODULE5);
    controller_module_disable_i2c(MODULE6);
    controller_module_disable_i2c(MODULE7);
}

void controller_all_modules_disable_usb (void) {
    controller_module_disable_usb(MODULE0);
    controller_module_disable_usb(MODULE1);
    controller_module_disable_usb(MODULE2);
    controller_module_disable_usb(MODULE5);
    controller_module_disable_usb(MODULE6);
    controller_module_disable_usb(MODULE7);
}
