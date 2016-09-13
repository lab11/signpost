#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <gpio_async.h>

#define MOD0_GPIOA_PORT_NUM 0
#define MOD1_GPIOA_PORT_NUM 1
#define MOD2_GPIOA_PORT_NUM 2
#define MOD5_GPIOA_PORT_NUM 3
#define MOD6_GPIOA_PORT_NUM 4
#define MOD7_GPIOA_PORT_NUM 5

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _pin_value;

// Callback when the pressure reading is ready
void gpioa_callback (int callback_type, int pin_value, int unused, void* callback_args) {
  UNUSED_PARAMETER(unused);
  UNUSED_PARAMETER(callback_args);

  _pin_value = pin_value;
}

int main () {
  putstr("Welcome to Tock...extend the GPIO boom\n");

  // Pass a callback function to the kernel
  gpio_async_set_callback(gpioa_callback, NULL);

  gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 0);
  wait();

  gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 1);
  wait();

  gpio_async_set(MOD7_GPIOA_PORT_NUM, 0);
  wait();
  // gpio_async_clear(10, 0);
  // wait();
  // gpio_async_toggle(10, 0);
  // wait();
  // gpio_async_toggle(10, 0);
  // wait();

  {
    // Print the pressure value
    char buf[64];
    sprintf(buf, "\tSet Some GPIO async\n\n");
    putstr(buf);
  }

  // while (1) {

  //   int i;
  //   for (i=0;i<10;i++) {
  //     gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 2);
  //     wait();
  //     gpio_async_clear(MOD7_GPIOA_PORT_NUM, 2);
  //     wait();
  //     gpio_async_set(MOD7_GPIOA_PORT_NUM, 2);
  //     // gpio_async_enable_input(10, 2, PullNone);
  //     wait();
  //     delay_ms(10);
  //   }



  //   // delay_ms(500);
  //   gpio_async_toggle(MOD7_GPIOA_PORT_NUM, 1);
  //   wait();
  // }

  // gpio_async_enable_input(10, 0, PullNone);
  // wait();

  // while (1) {
  //   gpio_async_read(10, 0);

  //   wait();

  //   {
  //     // Print the pressure value
  //     char buf[64];
  //     sprintf(buf, "\tREAD GPIO async %i\n\n", _pin_value);
  //     putstr(buf);
  //   }
  // }
}