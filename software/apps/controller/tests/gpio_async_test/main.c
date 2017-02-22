#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "gpio_async.h"


int main (void) {
  putstr("[GPIO Async] Test\n");

  // Enable some outputs
  gpio_async_enable_output_sync(0, 0);
  gpio_async_enable_output_sync(0, 1);

  // Set one high
  gpio_async_set_sync(0, 0);

  {
    char buf[64];
    sprintf(buf, "Set Some GPIO async\n");
    putstr(buf);
  }
}
