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
  int current;

  signpost_energy_init_ltc2943(NULL);

  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();

  int i;

  while (1) {

    printf("\nChecking Energy\n");

    for (i=0; i<8; i++) {
      if (i == 3 || i == 4) continue;

        energy = signpost_energy_get_module_energy(i);
        current = signpost_energy_get_module_current(i);
        printf("Module %i energy: %i uAh\t\tcurrent: %i uA\n", i, energy, current);
    }

    energy = signpost_energy_get_controller_energy();
    current = signpost_energy_get_controller_current();
    printf("Controller energy: %i uAh\tcurrent: %i uA\n", energy, current);

    energy = signpost_energy_get_linux_energy();
    current = signpost_energy_get_linux_current();
    printf("Linux energy: %i uAh\t\tcurrent: %i uA\n", energy, current);

    int v = signpost_energy_get_battery_voltage();
    int c = signpost_energy_get_battery_current();
    int e = signpost_energy_get_battery_energy_remaining();

    int s_voltage = signpost_energy_get_solar_voltage();
    int s_current = signpost_energy_get_solar_current();

    printf("Battery Voltage (mV): %d\tcurrent (uA): %d\tenergy (uAh):%d\n",(int)v,(int)c,(int)e);
    printf("Solar Voltage (mV): %d\tcurrent (uA): %d\n",s_voltage,s_current);

    delay_ms(1000);
  }
}
