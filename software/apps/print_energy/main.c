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

static void gpio_async_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}

void print_data (int module, int energy) {
  char buf[64];
  if (module == 3) {
    sprintf(buf, "Controller energy: %i\n", energy);
  } else if (module == 4) {
    sprintf(buf, "Linux energy: %i\n", energy);
  } else {
    sprintf(buf, "Module %i energy: %i\n", module, energy);
  }
  putstr(buf);
}

int main () {
  int energy;

  signpost_energy_init();

  gpio_async_set_callback(gpio_async_callback, NULL);
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();


  int i;

  while (1) {

    putstr("\nChecking Energy\n");

    for (i=0; i<8; i++) {
      if (i == 3 || i == 4) continue;

      energy = signpost_energy_get_module_energy(i);
      print_data(i, energy);
    }

    energy = signpost_energy_get_controller_energy();
    print_data(3, energy);

    energy = signpost_energy_get_linux_energy();
    print_data(4, energy);


    delay_ms(500);
  }









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

  while(1) {
    yield();
  }
}
