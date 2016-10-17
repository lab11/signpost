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


////////
// GPIOs
#define MOD0_IN    PA04
#define MOD1_IN    PA05
#define MOD2_IN    PA06
#define MOD5_IN    PA07
#define MOD6_IN    PA08
#define MOD7_IN    PA09
#define MOD0_OUT   PA13
#define MOD1_OUT   PA14
#define MOD2_OUT   PA15
#define MOD5_OUT   PA16
#define MOD6_OUT   PA17
#define MOD7_OUT   PA18

// Enum indicies must match array in <tock>/kernel/boards/controller/src/main.rs
enum GPIO_Pin_enum{
    PA04=0,
    PA05,
    PA06,
    PA07,
    PA08,
    PA09,
    PA13,
    PA14,
    PA15,
    PA16,
    PA17,
    PA18,
    PA26,
};

void controller_init_module_switches ();

void controller_gpio_enable_all_module_inputs (GPIO_InputMode_t pin_config);
void controller_gpio_enable_all_module_outputs ();
void controller_gpio_set_all ();
void controller_gpio_clear_all ();

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
