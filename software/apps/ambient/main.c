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
#include "app_watchdog.h"

uint8_t txbuf[32] = {0};

#define _U __attribute__ ((unused))

static void print_measurements (int temp, int humi, int pres, int ligh) {
  char buf[256];
  putstr("[Ambient] Got Measurements\n");

  // Temperature and Humidity
  sprintf(buf, "  Temp(%d 1/100 degrees C) [0x%X]\n", temp, temp);
  putstr(buf);
  sprintf(buf, "  Humi(%d 0.01%%) [0x%X]\n", humi, humi);
  putstr(buf);

  // Print the pressure value
  sprintf(buf, "  Pressure(%d ubar) [0x%X]\n", pres, pres);
  putstr(buf);

  // Light
  sprintf(buf, "  Light(%d) [0x%X]\n", ligh, ligh);
  putstr(buf);
}

static void sample_and_send (void) {
  // Start a pressure measurement
  int pressure = lps331ap_get_pressure_sync();
  // Get light
  int light = isl29035_read_light_intensity();
  // Get temperature and humidity
  int temperature;
  unsigned humidity;
  si7021_get_temperature_humidity_sync(&temperature, &humidity);

  // Encode readings in txbuf
  txbuf[2] = (uint8_t) ((temperature >> 8) & 0xFF);
  txbuf[3] = (uint8_t) (temperature & 0xFF);
  txbuf[4] = (uint8_t) ((humidity >> 8) & 0xFF);
  txbuf[5] = (uint8_t) (humidity & 0xFF);
  txbuf[6] = (uint8_t) ((light >> 8) & 0xFF);
  txbuf[7] = (uint8_t) (light & 0xFF);
  txbuf[8] = (uint8_t) ((pressure >> 16) & 0xFF);
  txbuf[9] = (uint8_t) ((pressure >> 8) & 0xFF);
  txbuf[10] = (uint8_t) (pressure & 0xFF);

  print_measurements(temperature, humidity, pressure, light);

  int result = i2c_master_slave_write_sync(0x22, 11);
  if (result >= 0) {
    app_watchdog_tickle_kernel();
    led_toggle(0);
  } else {
    char buf[256];
    sprintf(buf, "I2C Write: %i\n", result);
    putstr(buf);
  }
}

static void timer_callback (int callback_type _U, int pin_value _U, int unused _U, void* callback_args _U) {
  sample_and_send();
}

int main (void) {
  putstr("[Ambient] Measure and Report\n");

  // Setup I2C TX buffer
  txbuf[0] = 0x32; // My address
  txbuf[1] = 0x01; // Message type

  // Set buffer for I2C messages
  i2c_master_slave_set_master_write_buffer(txbuf, 32);

  // Set our address in case anyone cares
  i2c_master_slave_set_slave_address(0x32);

  // sample immediately
  sample_and_send();

  // Setup a timer for sampling the sensors
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(6000);

  // Setup watchdog
  app_watchdog_set_kernel_timeout(10000);
  app_watchdog_start();
}
