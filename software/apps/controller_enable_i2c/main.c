#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "signpost_energy.h"
#include "controller.h"
#include "i2c_master_slave.h"



uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];





static void print_data (int length) {
  char buf[64];

  // Need at least two bytes to be a valid signpost message.
  if (length < 2) {
    return;
  }

  // First byte is the sender
  int sender_address = slave_write_buf[0];
  // Second byte is the packet type
  int message_type = slave_write_buf[1];

  // Handle each type of message.
  switch (sender_address) {
    case 0x31: { // 802.15.4 scanner
      if (message_type == 1 && length == (16+2)) {
        // Got valid message from 15.4 scanner
        putstr("Message type 1 from Scanner15.4\n");
        for (int channel=11; channel<27; channel++) {
          sprintf(buf, "  Channel %i RSSI: %i\n", channel, (int) ((int8_t) slave_write_buf[2+(channel-11)]));
          putstr(buf);
        }
      }

      break;
    }
    case 0x32: { // Ambient
      if (message_type == 1 && length == (8+2)) {
        // Got valid message from ambient
        putstr("Message type 1 from Ambient\n");
        int temp = (int) ((int16_t) ((((uint16_t) slave_write_buf[2]) << 8) | ((uint16_t) slave_write_buf[3])));
        int humi = (int) ((int16_t) ((((uint16_t) slave_write_buf[4]) << 8) | ((uint16_t) slave_write_buf[5])));
        int ligh = (int) ((int16_t) ((((uint16_t) slave_write_buf[6]) << 8) | ((uint16_t) slave_write_buf[7])));
        int pres = (int) ((int16_t) ((((uint16_t) slave_write_buf[8]) << 8) | ((uint16_t) slave_write_buf[9])));
        sprintf(buf, "  Temperature: %i 1/100 degrees C\n", temp);
        putstr(buf);
        sprintf(buf, "  Humidity: %i 0.01%%\n", humi);
        putstr(buf);
        sprintf(buf, "  Light: %i Lux\n", ligh);
        putstr(buf);
        sprintf(buf, "  Pressure: %i ubar\n", pres);
        putstr(buf);
      }

      break;
    }
    default: {
      sprintf(buf, "Different message? %i\n  ", sender_address);
      putstr(buf);
      for (int i=0; i<length; i++) {
        sprintf(buf, "0x%02x ", slave_write_buf[i]);
        putstr(buf);
      }
      sprintf(buf, "\n");
      putstr(buf);
    }
  }
}

static void i2c_master_slave_callback (
        int callback_type,
        int length,
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {

  if (callback_type == 3) {
    print_data(length);
  }
}

int main (void) {
  putstr("[Controller] Start!\n");

  // Setup backplane
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();
  // controller_all_modules_disable_i2c();
  // controller_module_enable_i2c(MODULE5);
  // controller_module_enable_i2c(MODULE0);
  putstr("Enabled\n");

  // Setup I2C listen
  i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
  i2c_master_slave_set_slave_address(0x20);

  i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
  i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
  i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
  i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

  i2c_master_slave_listen();
  putstr("Init complete\n");
}
