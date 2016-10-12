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

// // Callback when the pressure reading is ready
// void sensor_callback (int temperature, int humidity, int unused, void* callback_args) {
//   UNUSED_PARAMETER(unused);
//   UNUSED_PARAMETER(callback_args);

//   _temp_reading = temperature;
//   _humi_reading = humidity;
// }

int main () {
  gpio_enable_output(0);
  gpio_toggle(0);
  delay_ms(500);
  gpio_toggle(0);
  delay_ms(500);

  putstr("Welcome to Tock...lets get isl lighted\n");

  int light = isl29035_read_light_intensity();

  {
    // Print the value
    char buf[64];
    sprintf(buf, "\tLight(%d) [0x%X]\n\n", light, light);
    putstr(buf);
  }
}
