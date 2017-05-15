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
#include "signpost_energy_monitors.h"

int main (void) {
  int energy;
  int current;

  signpost_energy_init_ltc2943();

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
        printf("Module %i energy: %i uWh\t\tcurrent: %i uA\n", i, energy, current);
    }

    energy = signpost_energy_get_controller_energy();
    current = signpost_energy_get_controller_current();
    printf("Controller energy: %i uWh\tcurrent: %i uA\n", energy, current);

    energy = signpost_energy_get_linux_energy();
    current = signpost_energy_get_linux_current();
    printf("Linux energy: %i uWh\t\tcurrent: %i uA\n", energy, current);

    int v = signpost_energy_get_battery_voltage();
    int c = signpost_energy_get_battery_current();
    int e = signpost_energy_get_battery_energy();
    int p = signpost_energy_get_battery_percent();
    int cap = signpost_energy_get_battery_capacity();

    int s_voltage = signpost_energy_get_solar_voltage();
    int s_current = signpost_energy_get_solar_current();

    printf("Battery Voltage (mV): %d\tcurrent (uA): %d\tenergy (uWh):%d\n",(int)v,(int)c,(int)e);
    printf("Battery Percent: %d\tcapacity (uWh): %d\n",(int)p,(int)cap);
    printf("Solar Voltage (mV): %d\tcurrent (uA): %d\n",s_voltage,s_current);

    delay_ms(1000);
  }
}
