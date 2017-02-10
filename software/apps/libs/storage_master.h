#pragma once

#include "gpio.h"

////////
// GPIOs
#define EDISON_PWRBTN  PA05
#define LINUX_ENABLE_POWER PA06
#define STORAGE_LED    PA07

// Enum indicies must match array in <tock>/kernel/boards/storage_master/src/main.rs
enum GPIO_Pin_enum{
    PA05=0,
    PA06,
    PA07,
};

void storage_master_enable_edison (void);
void storage_master_wakeup_edison (void);
