#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tock.h>

#include "gpio_async.h"


int main (void) {
  printf("[GPIO Async] Test\n");

  // Enable some outputs
  gpio_async_enable_output_sync(0, 0);
  gpio_async_enable_output_sync(0, 1);

  // Set one high
  gpio_async_set_sync(0, 0);

  printf("Set Some GPIO async\n");
}
