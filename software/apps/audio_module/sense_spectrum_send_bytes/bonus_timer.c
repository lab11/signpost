#include <stdint.h>

#include <tock.h>

#include "bonus_timer.h"

int bonus_timer_subscribe(subscribe_cb cb, void *userdata) {
  return subscribe(203, 0, cb, userdata);
}

int bonus_timer_oneshot(uint32_t interval) {
  return command(203, 1, (int)interval);
}

int bonus_timer_start_repeating(uint32_t interval) {
  return command(203, 2, (int)interval);
}

int bonus_timer_stop(void) {
  return command(203, 3, 0);
}

unsigned int bonus_timer_read(void) {
  return (unsigned int)command(203, 4, 0);
}
