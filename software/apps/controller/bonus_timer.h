
#include <stdint.h>
#include "tock.h"


int bonus_timer_subscribe(subscribe_cb cb, void *userdata);
int bonus_timer_oneshot(uint32_t interval);
int bonus_timer_start_repeating(uint32_t interval);
int bonus_timer_stop(void);
unsigned int bonus_timer_read(void);
