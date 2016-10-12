#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <adc.h>

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

// Callback when the adc reading is done
void callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_args);

  _data = data;
  _data2 = data2;

}

void print_data () {
  char buf[64];
  // _data2 is left alligned to 16 bits
  sprintf(buf, "\tGot: 0x%02x\n\n", _data2>>4);
  putstr(buf);
}

int main () {
  putstr("Welcome to Tock...lets measure the ADC!!\n");

  adc_set_callback(callback, NULL);
  // initialize adc
  adc_initialize();

  while (1) {
    adc_single_sample(1);
    yield();
    print_data();
    delay_ms(2000);
  }

  // gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 0);
  // yield();

  // gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 1);
  // yield();

  // gpio_async_set(MOD7_GPIOA_PORT_NUM, 0);
  // yield();
  // gpio_async_clear(10, 0);
  // yield();
  // gpio_async_toggle(10, 0);
  // yield();
  // gpio_async_toggle(10, 0);
  // yield();



  // while (1) {

  //   int i;
  //   for (i=0;i<10;i++) {
  //     gpio_async_enable_output(MOD7_GPIOA_PORT_NUM, 2);
  //     yield();
  //     gpio_async_clear(MOD7_GPIOA_PORT_NUM, 2);
  //     yield();
  //     gpio_async_set(MOD7_GPIOA_PORT_NUM, 2);
  //     // gpio_async_enable_input(10, 2, PullNone);
  //     yield();
  //     delay_ms(10);
  //   }



  //   // delay_ms(500);
  //   gpio_async_toggle(MOD7_GPIOA_PORT_NUM, 1);
  //   yield();
  // }

  // gpio_async_enable_input(10, 0, PullNone);
  // yield();

  // while (1) {
  //   gpio_async_read(10, 0);

  //   yield();

  //   {
  //     // Print the pressure value
  //     char buf[64];
  //     sprintf(buf, "\tREAD GPIO async %i\n\n", _pin_value);
  //     putstr(buf);
  //   }
  // }
}
