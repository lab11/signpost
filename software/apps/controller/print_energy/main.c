#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <tock.h>
#include <timer.h>

#include "controller.h"
#include "i2c_selector.h"
#include "signpost_energy.h"

int main (void) {
  int energy;

  signpost_energy_init();

  signpost_energy_reset_all_energy();

  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();

  int i;

  while (1) {

    printf("\nChecking Energy\n");

    for (i=0; i<8; i++) {
      if (i == 3 || i == 4) continue;

        energy = signpost_energy_get_module_energy(i);
        printf("Module %i energy: %i uAh\n", i, energy);
    }

    energy = signpost_energy_get_controller_energy();
    printf("Controller energy: %i uAh\n", energy);

    energy = signpost_energy_get_linux_energy();
    printf("Linux energy: %i uAh\n", energy);

    delay_ms(1000);
  }
}
