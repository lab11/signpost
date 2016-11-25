#include "gpio.h"
#include "storage_master.h"

void storage_master_enable_edison () {
    gpio_enable_output(EDISON_PWRBTN);
}

void storage_master_wakeup_edison () {
    gpio_clear(EDISON_PWRBTN);
    for(volatile uint8_t i = 0; i < 1000; i++);
    gpio_set(EDISON_PWRBTN);
}
