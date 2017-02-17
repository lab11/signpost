#pragma once

#include "gpio.h"

////////
// GPIOs
// Enum indicies must match array in <tock>/kernel/boards/storage_master/src/main.rs
#define EDISON_PWRBTN PA05
#define LINUX_ENABLE_POWER PA06
#define SD_ENABLE PA21
enum GPIO_Pin_enum {
    PA05=0,
    PA06,
    PA21,
};

////////
// LEDs
// Enum indicies must match array in <tock>/kernel/boards/storage_master/src/main.rs
#define DEBUG_LED0 PB04
#define DEBUG_LED1 PB05
#define STORAGE_LED PA07
enum LED_Pin_enum {
    PB04=0,
    PB05,
    PA07,
};

void storage_master_enable_edison (void);
void storage_master_wakeup_edison (void);
