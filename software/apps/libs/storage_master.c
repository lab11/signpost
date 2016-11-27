#include "gpio.h"
#include "storage_master.h"

void storage_master_enable_edison () {
    gpio_enable_output(EDISON_PWRBTN);
    gpio_set(EDISON_PWRBTN);
}

void storage_master_wakeup_edison () {
    gpio_clear(EDISON_PWRBTN);
    //this is about 100ms. 10ms didn't work. Did try other values
    for(volatile uint32_t i = 0; i < 100000; i++);
    gpio_set(EDISON_PWRBTN);
}
