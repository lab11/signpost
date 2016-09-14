#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <ltc2941.h>

#define MOD0_GPIOA_PORT_NUM 0
#define MOD1_GPIOA_PORT_NUM 1
#define MOD2_GPIOA_PORT_NUM 2
#define MOD5_GPIOA_PORT_NUM 3
#define MOD6_GPIOA_PORT_NUM 4
#define MOD7_GPIOA_PORT_NUM 5

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _data = 5;
int _data2 = 6;

// Callback when the pressure reading is ready
void callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_args);

  _data = data;
  _data2 = data2;

}

void print_data () {
  char buf[64];
  sprintf(buf, "\tGot something: 0x%02x  | 0x%02x\n\n", _data, _data2);
  putstr(buf);
}

int main () {
  putstr("Welcome to Tock...lets count some coulombs!!\n");

  // Pass a callback function to the kernel
  ltc2941_set_callback(callback, NULL);

  ltc2941_read_status();
  wait();
  print_data();

  ltc2941_reset_charge();
  wait();

  ltc2941_set_high_threshold(0x0010);
  wait();

  while (1) {
    ltc2941_get_charge();
    wait();
    print_data();
    delay_ms(1000);
  }

  // gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 0);
  // wait();

  // gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 1);
  // wait();

  // gpio_async_set(MOD7_GPIOA_PORT_NUM, 0);
  // wait();
  // gpio_async_clear(10, 0);
  // wait();
  // gpio_async_toggle(10, 0);
  // wait();
  // gpio_async_toggle(10, 0);
  // wait();



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