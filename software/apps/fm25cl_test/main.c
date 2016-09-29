#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <fm25cl.h>

// #define MOD0_GPIOA_PORT_NUM 0
// #define MOD1_GPIOA_PORT_NUM 1
// #define MOD2_GPIOA_PORT_NUM 2
// #define MOD5_GPIOA_PORT_NUM 3
// #define MOD6_GPIOA_PORT_NUM 4
// #define MOD7_GPIOA_PORT_NUM 5

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _data = 5;
int _data2 = 6;

uint8_t read_buf[256];
uint8_t write_buf[256];


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

void print_buf () {
  char buf[64];
  sprintf(buf, "\tData: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n\n", read_buf[0], read_buf[1], read_buf[2], read_buf[3], read_buf[4]);
  putstr(buf);
}

int main () {
  putstr("Welcome to Tock...lets store the datas\n");

  // Pass a callback function to the kernel
  fm25cl_set_callback(callback, NULL);

  fm25cl_set_read_buffer(read_buf, 256);
  fm25cl_set_write_buffer(write_buf, 256);

  fm25cl_read_status();
  yield();
  print_data();

  write_buf[0] = 0x02;
  write_buf[1] = 0x06;
  write_buf[2] = 0x0a;
  write_buf[3] = 0x0e;
  write_buf[4] = 0x9f;

  fm25cl_write(0x24, 5);
  yield();

  fm25cl_read(0x24, 5);
  yield();

  print_buf();

  fm25cl_read_status();
  yield();
  print_data();

  fm25cl_read_status();
  yield();
  print_data();

  fm25cl_read_status();
  yield();
  print_data();

  fm25cl_read_status();
  yield();
  print_data();

  fm25cl_read_status();
  yield();
  print_data();



  // ltc2941_read_status();
  // yield();
  // print_data();

  // ltc2941_reset_charge();
  // yield();

  // ltc2941_set_high_threshold(0x0010);
  // yield();

  // while (1) {
  //   ltc2941_get_charge();
  //   yield();
  //   print_data();
  //   delay_ms(1000);
  // }

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
