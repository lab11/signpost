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

static void print_data (int module, int energy, int current) {
  int int_energy = signpost_ltc_to_uAh(energy, POWER_MODULE_RSENSE);
  if (module == 3) {
    printf("Controller energy: %i uAh\tcurrent: %i uA\n", int_energy, current);
  } else if (module == 4) {
    printf("Linux energy: %i uAh\t\tcurrent: %i uA\n", int_energy, current);
  } else {
    printf("Module %i energy: %i uAh\t\tcurrent: %i uA\n", module, int_energy, current);
  }
}

int main (void) {
  int energy;
  int current;

  signpost_energy_init_ltc2943();

  signpost_energy_reset();

  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();

  int i;

  while (1) {

    printf("\nChecking Energy\n");

    for (i=0; i<8; i++) {
      if (i == 3 || i == 4) continue;

      energy = signpost_energy_get_module_energy(i);
      current = signpost_energy_get_module_current_ua(i);
      print_data(i, energy, current);
    }

    energy = signpost_energy_get_controller_energy();
    current = signpost_energy_get_controller_current_ua();
    print_data(3, energy, current);

    energy = signpost_energy_get_linux_energy();
    current = signpost_energy_get_linux_current_ua();
    print_data(4, energy, current);

    int v = signpost_energy_get_battery_voltage_mv();
    int c = signpost_energy_get_battery_current_ua();
    int e = signpost_energy_get_battery_energy();
    int p = signpost_energy_get_battery_percent();
    int cap = signpost_energy_get_battery_capacity();

    int s_voltage = signpost_energy_get_solar_voltage_mv();
    int s_current = signpost_energy_get_solar_current_ua();

    printf("Battery Voltage (mV): %d\tcurrent (uA): %d\tenergy (uAh):%d\n",(int)v,(int)c,(int)e);
    printf("Battery Percent: %d\tcapacity (uAh): %d\n",(int)p,(int)cap);
    printf("Solar Voltage (mV): %d\tcurrent (uA): %d\n",s_voltage,s_current);

    delay_ms(1000);
  }
}
