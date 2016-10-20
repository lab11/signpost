#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "lps331ap.h"
#include "i2c_master_slave.h"
#include "isl29035.h"
#include "si7021.h"
#include "timer.h"
#include "led.h"

uint8_t txbuf[32] = {0};

#define _U __attribute__ ((unused))

void print_measurements (int temp, int humi, int pres, int ligh) {
  char buf[256];
  putstr("[Ambient] Got Measurements\n");

  // Temperature and Humidity
  sprintf(buf, "\tTemp(%d 1/100 degrees C) [0x%X]\n", temp, temp);
  putstr(buf);
  sprintf(buf, "\tHumi(%d 0.01%%) [0x%X]\n", humi, humi);
  putstr(buf);

  // Print the pressure value
  sprintf(buf, "\tPressure(%d ubar) [0x%X]\n", pres, pres);
  putstr(buf);

  // Light
  sprintf(buf, "\tLight(%d) [0x%X]\n", ligh, ligh);
  putstr(buf);
}

void sample_and_send () {
  // Start a pressure measurement
  int pressure = lps331ap_get_pressure_sync();
  // Get light
  int light = isl29035_read_light_intensity();
  // Get temperature and humidity
  int temperature, humidity;
  si7021_get_temperature_humidity_sync(&temperature, &humidity);

  // Encode readings in txbuf
  txbuf[2] = (uint8_t) ((temperature >> 8) & 0xFF);
  txbuf[3] = (uint8_t) (temperature & 0xFF);
  txbuf[4] = (uint8_t) ((humidity >> 8) & 0xFF);
  txbuf[5] = (uint8_t) (humidity & 0xFF);
  txbuf[6] = (uint8_t) ((light >> 8) & 0xFF);
  txbuf[7] = (uint8_t) (light & 0xFF);
  txbuf[8] = (uint8_t) ((pressure >> 8) & 0xFF);
  txbuf[9] = (uint8_t) (pressure & 0xFF);

  print_measurements(temperature, humidity, pressure, light);

  // i2c_master_slave_write_sync(0x20, 10);

  led_toggle(0);
}

static void timer_callback (int callback_type _U, int pin_value _U, int unused _U, void* callback_args _U) {
  sample_and_send();
}

int main () {
  putstr("[Ambient] Measure and Report\n");

  // Setup I2C TX buffer
  txbuf[0] = 0x32; // My address
  txbuf[1] = 0x01; // Message type

  // Set buffer for I2C messages
  i2c_master_slave_set_master_write_buffer(txbuf, 32);

  // Set our address in case anyone cares
  i2c_master_slave_set_slave_address(0x32);

  // Setup a timer for sampling the sensors
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(6000);
}
