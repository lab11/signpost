#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "firestorm.h"
#include "tock.h"
#include "tock_str.h"
#include "signpost_energy.h"

// #define MOD0_GPIOA_PORT_NUM 0
// #define MOD1_GPIOA_PORT_NUM 1
// #define MOD2_GPIOA_PORT_NUM 2
// #define MOD5_GPIOA_PORT_NUM 3
// #define MOD6_GPIOA_PORT_NUM 4
// #define MOD7_GPIOA_PORT_NUM 5

// #define UNUSED_PARAMETER(x) (void)(x)

// // Global store
// int _data = 5;
// int _data2 = 6;

// // Callback when the pressure reading is ready
// void callback (int callback_type, int data, int data2, void* callback_args) {
//   UNUSED_PARAMETER(callback_args);

//   _data = data;
//   _data2 = data2;

// }

int _length;
int _go = 0;

uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];

static void gpio_async_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}

static void i2c_master_slave_callback (
        int callback_type,
        int length,
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {

  if (callback_type == 3) {
    _length = length;

    _go = 1;
  }
}

void print_data () {
  char buf[64];
  sprintf(buf, "I2C Slave: %i (%02x %02x %02x %02x %02x %02x)\n", _length, slave_write_buf[0],
                                                                 slave_write_buf[1],
                                                                 slave_write_buf[2],
                                                                 slave_write_buf[3],
                                                                 slave_write_buf[4],
                                                                 slave_write_buf[5]);
  putstr(buf);
}

int main () {
  putstr("[Controller] Start!\n");
  // int energy;

  // signpost_energy_init();

  gpio_async_set_callback(gpio_async_callback, NULL);
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();

  i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
  i2c_master_slave_set_slave_address(0x02);

  i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
  i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
  i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
  i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

  i2c_master_slave_listen();



  while (1) {
    yield();

    if (_go == 1) {
      _go = 0;

      print_data();
    }
  }


  // int i;

  // while (1) {

  //   putstr("\nChecking Energy\n");

  //   for (i=0; i<8; i++) {
  //     if (i == 3 || i == 4) continue;

  //     energy = signpost_energy_get_module_energy(i);
  //     print_data(i, energy);
  //   }

  //   energy = signpost_energy_get_controller_energy();
  //   print_data(3, energy);

  //   energy = signpost_energy_get_linux_energy();
  //   print_data(4, energy);


  //   delay_ms(500);
  // }









  // putstr("Welcome to Tock...lets wait for an interrupt!!\n");


  // // Pass a callback function to the kernel
  // smbus_interrupt_set_callback(callback, NULL);
  // i2c_selector_set_callback(callback, NULL);
  // ltc2941_set_callback(callback, NULL);

  // // Read from first selector, chan
  // i2c_selector_select_channels(0x01);
  // yield();

  // // // Reset charge
  // // ltc2941_reset_charge();
  // // yield();
  // // And tell it any interrupt that may have existed has been handled
  // smbus_interrupt_issue_alert_response();
  // yield();

  // // Read status, first byte should be 0x00
  // int carltc2941_get_charge();
  // yield();
  // print_data(0);

  // // // Set high threshold really low so we get a fast interrupt
  // // ltc2941_set_high_threshold(0x0000);
  // // yield();

  // // // Open all channels so any gauge can interrupt and respond to SMBUS Alert Response
  // // // Even though all gauges share same address, and multiple may be interrupting
  // // // they will all respond with the same address
  // // i2c_selector_select_channels(0xFF);

  // // // Wait for interrupt, then print address of who interrupted
  // // // Should be 0xc8 (gauge address with a 1 tacked on the end)
  // // putstr("Waiting...\n");
  // // yield();
  // // print_data(0);

  // // putstr("Reading Interrupts\n");

  // // // Query the i2c_selector driver for who interrupted
  // // i2c_selector_read_interrupts();
  // // yield();
  // // print_data(0);

  // // i2c_selector_select_channels(_data2);
  // // yield();

  // // // Reset charge and handle interrupt
  // // ltc2941_reset_charge();
  // // yield();
  // // smbus_interrupt_issue_alert_response();
  // // yield();

  // // // Read status twice
  // // ltc2941_read_status();
  // // yield();
  // // print_data(0);
  // // ltc2941_read_status();
  // // yield();
  // // print_data(0);

  // while(1) {
  //   yield();
  // }
}
