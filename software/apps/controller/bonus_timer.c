
#include <stdint.h>
#include <tock.h>


int bonus_timer_subscribe(subscribe_cb cb, void *userdata) {
  return subscribe(203, 0, cb, userdata);
}

int bonus_timer_oneshot(uint32_t interval) {
  return command(203, 0, (int)interval);
}

int bonus_timer_start_repeating(uint32_t interval) {
  return command(203, 1, (int)interval);
}

int bonus_timer_stop() {
  return command(203, 2, 0);
}

unsigned int bonus_timer_read(){
  return (unsigned int)command(203, 3, 0);
}