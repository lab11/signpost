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

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _temp_reading;
int _humi_reading;

uint8_t txbuf[32] = {0};

int main () {
  putstr("[Ambient] Measure and Report8\n");

  // Setup I2C TX buffer
  txbuf[0] = 0x32; // My address
  txbuf[1] = 0x01; // Message type

  // Set buffer for I2C messages
  i2c_master_slave_set_master_write_buffer(txbuf, 32);

  // Set our address in case anyone cares
  i2c_master_slave_set_slave_address(0x32);

  while (1) {
    // Start a pressure measurement
    // Not working at the moment
    int pressure = lps331ap_get_pressure_sync();

    int light = isl29035_read_light_intensity();

    // Start a measurement
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

    // i2c_master_slave_write_sync(0x20, 10);

    {
      char buf[256];
      putstr("[Ambient] Got Measurements\n");

      // Temperature and Humidity
      sprintf(buf, "\tTemp(%d 1/100 degrees C) [0x%X]\n", temperature, temperature);
      putstr(buf);
      sprintf(buf, "\tHumi(%d 0.01%%) [0x%X]\n", humidity, humidity);
      putstr(buf);

      // Print the pressure value
      sprintf(buf, "\tPressure(%d ubar) [0x%X]\n", pressure, pressure);
      putstr(buf);

      // Light
      sprintf(buf, "\tLight(%d) [0x%X]\n", light, light);
      putstr(buf);
    }

    // Delay
    delay_ms(5000);
  }
}
