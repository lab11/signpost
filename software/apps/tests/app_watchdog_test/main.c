#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "app_watchdog.h"
#include "timer.h"

int counter = 0;

static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
  counter++;

  if (counter < 15) {
    app_watchdog_tickle_kernel();
    putstr("[Watchdog - app] Tickled.\n");
  } else {
    putstr("[Watchdog - app] Not going to tickle, we should restart shortly.\n");
  }
}

int main () {
  putstr("[Watchdog - app] Test\n");

  // Need a timer
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(750);

  app_watchdog_set_app_timeout(1200);
  app_watchdog_start();
}
