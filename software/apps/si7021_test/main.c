#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <firestorm.h>
#include <si7021.h>

#define UNUSED_PARAMETER(x) (void)(x)

// Global store
int _temp_reading;
int _humi_reading;

// Callback when the pressure reading is ready
void sensor_callback (int temperature, int humidity, int unused, void* callback_args) {
  UNUSED_PARAMETER(unused);
  UNUSED_PARAMETER(callback_args);

  _temp_reading = temperature;
  _humi_reading = humidity;
}

int main () {
  gpio_enable_output(0);
  gpio_toggle(0);
  delay_ms(500);
  gpio_toggle(0);
  delay_ms(500);

  putstr("Welcome to Tock...lets get tempified2\n");

  // Pass a callback function to the kernel
  si7021_set_callback(sensor_callback, NULL);

  // Start a measurement
  si7021_get_temperature_humidity();

  yield();

  {
    // Print the value
    char buf[64];
    sprintf(buf, "\tTemp(%d 1/100 degrees C) [0x%X]\n\n", _temp_reading, _temp_reading);
    putstr(buf);
    sprintf(buf, "\tHumi(%d 0.01%%) [0x%X]\n\n", _humi_reading, _humi_reading);
    putstr(buf);
  }
}
