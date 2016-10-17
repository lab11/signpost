#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "tock.h"
#include "console.h"
#include "lps331ap.h"

int main () {
  putstr("[LP331AP] Test\n");

  // Start a pressure measurement
  int pressure = lps331ap_get_pressure_sync();

  {
    // Print the pressure value
    char buf[64];
    sprintf(buf, "\tValue(%d ubar) [0x%X]\n\n", pressure, pressure);
    putstr(buf);
  }
}
