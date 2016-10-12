#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <lps331ap.h>

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _pressure_reading = 0;
int _temp_reading;
int _humi_reading;

uint8_t txbuf[32] = {0};

// Callback when the pressure reading is ready
void temphum_callback (int temperature, int humidity, int unused, void* callback_args) {
  UNUSED_PARAMETER(unused);
  UNUSED_PARAMETER(callback_args);

  _temp_reading = temperature;
  _humi_reading = humidity;
}

// Callback when the pressure reading is ready
void pressure_callback (int pressure_value, int error_code, int unused, void* callback_args) {
  UNUSED_PARAMETER(error_code);
  UNUSED_PARAMETER(unused);
  UNUSED_PARAMETER(callback_args);

  _pressure_reading = pressure_value;
}

// Callback when the pressure reading is ready
static void i2c_master_slave_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}
static void putstrasync_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}

int main () {
  // char buff[256];
  // sprintf(buff, "Hello Branden\n");
  // putstr_async(buff, putstrasync_callback, NULL);
  // delay_ms(10);


  putstr("[Ambient] Measure and Report8\n");
  // putstr("[Ambient] Me\nasure and Report3\n", putstrasync_callback, NULL);
  // putstr_async("[Ambient] Me\nasure and Report4\n", putstrasync_callback, NULL);
  // yield();
  // delay_ms(10);

  // Setup I2C TX buffer
  txbuf[0] = 0x32; // My address
  txbuf[1] = 0x01; // Message type

  // Pass a callback function to the kernel for sensor readings
  lps331ap_set_callback(pressure_callback, NULL);
  si7021_set_callback(temphum_callback, NULL);

  // Set buffer for I2C messages
  i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
  i2c_master_slave_set_master_write_buffer(txbuf, 32);

  // Set our address in case anyone cares
  i2c_master_slave_set_slave_address(0x32);

  while (1) {
    // Start a pressure measurement
    // Not working at the moment
    // lps331ap_get_pressure();
    // yield();

    int light = isl29035_read_light_intensity();

    // Start a measurement
    si7021_get_temperature_humidity();
    yield();

    // Encode readings in txbuf
    txbuf[2] = (uint8_t) ((_temp_reading >> 8) & 0xFF);
    txbuf[3] = (uint8_t) (_temp_reading & 0xFF);
    txbuf[4] = (uint8_t) ((_humi_reading >> 8) & 0xFF);
    txbuf[5] = (uint8_t) (_humi_reading & 0xFF);
    txbuf[6] = (uint8_t) ((light >> 8) & 0xFF);
    txbuf[7] = (uint8_t) (light & 0xFF);
    txbuf[8] = (uint8_t) ((_pressure_reading >> 8) & 0xFF);
    txbuf[9] = (uint8_t) (_pressure_reading & 0xFF);

    i2c_master_slave_write(0x20, 10);
    yield();

    {
      char buf[256];
      putstr("[Ambient] Got Measurements\n");

      // Temperature and Humidity
      sprintf(buf, "\tTemp(%d 1/100 degrees C) [0x%X]\n", _temp_reading, _temp_reading);
      putstr(buf);
      sprintf(buf, "\tHumi(%d 0.01%%) [0x%X]\n", _humi_reading, _humi_reading);
      putstr(buf);

      // Print the pressure value
      sprintf(buf, "\tPressure(%d ubar) [0x%X]\n", _pressure_reading, _pressure_reading);
      putstr(buf);

      // Light
      sprintf(buf, "\tLight(%d) [0x%X]\n", light, light);
      putstr(buf);
    }

    // {
    //   char buf[256];
    //   // putstr_async("[Ambient] Got Measurements\n", putstrasync_callback, NULL);
    //   // yield();

    //   // Temperature and Humidity
    //   sprintf(buf, "\tTemp(%d 1/100 degrees C) [0x%X]\n", _temp_reading, _temp_reading);
    //   putstr_async(buf, putstrasync_callback, NULL);
    //   // yield();
    //   delay_ms(10);
    //   sprintf(buf, "\tHumi(%d 0.01%%) [0x%X]\n", _humi_reading, _humi_reading);
    //   putstr_async(buf, putstrasync_callback, NULL);
    //   // yield();
    //   delay_ms(10);

    //   // Print the pressure value
    //   sprintf(buf, "\tPressure(%d ubar) [0x%X]\n", _pressure_reading, _pressure_reading);
    //   putstr_async(buf, putstrasync_callback, NULL);
    //   // yield();
    //   delay_ms(10);

    //   // Light
    //   sprintf(buf, "\tLight(%d) [0x%X]\n", light, light);
    //   putstr_async(buf, putstrasync_callback, NULL);
    //   // yield();
    //   delay_ms(10);
    // }

    // Delay
    delay_ms(5000);
  }
}
