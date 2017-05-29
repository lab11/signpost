#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tock.h>

#include "i2c_selector.h"
#include "ltc294x.h"
#include "smbus_interrupt.h"

#define MOD0_GPIOA_PORT_NUM 0
#define MOD1_GPIOA_PORT_NUM 1
#define MOD2_GPIOA_PORT_NUM 2
#define MOD5_GPIOA_PORT_NUM 3
#define MOD6_GPIOA_PORT_NUM 4
#define MOD7_GPIOA_PORT_NUM 5

// Global store
int _data = 5;
int _data2 = 6;

// Callback when the pressure reading is ready
static void callback (
    int callback_type __attribute__ ((unused)),
    int data,
    int data2,
    void* callback_args __attribute__ ((unused))) {
  _data = data;
  _data2 = data2;
}

static void print_data (int i) {
  printf("\tGot something from counter %i: 0x%02x  | 0x%02x\n\n",i, _data, _data2);
}

int main (void) {
  printf("Welcome to Tock...lets wait for an interrupt!!\n");


  // Pass a callback function to the kernel
  smbus_interrupt_set_callback(callback, NULL);
  i2c_selector_set_callback(callback, NULL);
  ltc294x_set_callback(callback, NULL);

  // Read from first selector, channel 3 (1000)
  i2c_selector_select_channels(0x08);
  yield();

  // Reset charge
  ltc294x_reset_charge();
  yield();
  // And tell it any interrupt that may have existed has been handled
  smbus_interrupt_issue_alert_response();
  yield();

  // Read status, first byte should be 0x00
  ltc294x_read_status();
  yield();
  print_data(0);

  // Set high threshold really low so we get a fast interrupt
  ltc294x_set_high_threshold(0x0000);
  yield();

  // Open all channels so any gauge can interrupt and respond to SMBUS Alert Response
  // Even though all gauges share same address, and multiple may be interrupting
  // they will all respond with the same address
  i2c_selector_select_channels(0xFF);

  // Wait for interrupt, then print address of who interrupted
  // Should be 0xc8 (gauge address with a 1 tacked on the end)
  printf("Waiting...\n");
  yield();
  print_data(0);

  printf("Reading Interrupts\n");

  // Query the i2c_selector driver for who interrupted
  i2c_selector_read_interrupts();
  yield();
  print_data(0);

  i2c_selector_select_channels(_data2);
  yield();

  // Reset charge and handle interrupt
  ltc294x_reset_charge();
  yield();
  smbus_interrupt_issue_alert_response();
  yield();

  // Read status twice
  ltc294x_read_status();
  yield();
  print_data(0);
  ltc294x_read_status();
  yield();
  print_data(0);
}
