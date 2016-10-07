#pragma once

#define MOD0_GPIO_ASYNC_PORT_NUM 0
#define MOD1_GPIO_ASYNC_PORT_NUM 1
#define MOD2_GPIO_ASYNC_PORT_NUM 2
#define MOD5_GPIO_ASYNC_PORT_NUM 3
#define MOD6_GPIO_ASYNC_PORT_NUM 4
#define MOD7_GPIO_ASYNC_PORT_NUM 5

#define PIN_IDX_ISOLATE_POWER   0
#define PIN_IDX_ISOLATE_I2C     1
#define PIN_IDX_ISOLATE_USB     2

typedef enum {
    MODULE0 = 0,
    MODULE1 = 1,
    MODULE2 = 2,
    MODULE5 = 5,
    MODULE6 = 6,
    MODULE7 = 7,
} module_num_t;


void controller_init_module_switches ();

void controller_module_enable_power (module_num_t module_number);
void controller_module_enable_i2c (module_num_t module_number);
void controller_module_enable_usb (module_num_t module_number);

void controller_all_modules_enable_power ();
void controller_all_modules_enable_i2c ();
void controller_all_modules_enable_usb ();

void controller_module_disable_power (module_num_t module_number);
void controller_module_disable_i2c (module_num_t module_number);
void controller_module_disable_usb (module_num_t module_number);

void controller_all_modules_disable_power ();
void controller_all_modules_disable_i2c ();
void controller_all_modules_disable_usb ();
