#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "module.h"
#include "timer.h"
#include "tock.h"

int counter = 0;

static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}

int main (void) {
  putstr("[Test] API: Energy\n");

  // Need a timer
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(750);
}
