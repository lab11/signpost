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
int _pressure_reading;

// Callback when the pressure reading is ready
void pressure_callback (int pressure_value, int error_code, int unused, void* callback_args) {
  UNUSED_PARAMETER(error_code);
  UNUSED_PARAMETER(unused);
  UNUSED_PARAMETER(callback_args);

  _pressure_reading = pressure_value;
}

int main () {
  putstr("Welcome to Tock...lets get pressured\n");

  // Pass a callback function to the kernel
  lps331ap_set_callback(pressure_callback, NULL);

  // Start a pressure measurement
  lps331ap_get_pressure();

  yield();

  {
    // Print the pressure value
    char buf[64];
    sprintf(buf, "\tValue(%d ubar) [0x%X]\n\n", _pressure_reading, _pressure_reading);
    putstr(buf);
  }
}
