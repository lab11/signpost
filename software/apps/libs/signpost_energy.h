#pragma once

// qlsb = 0.0625mAh with 0.017 Ohm sense resistor
#define POWER_MODULE_PRESCALER 32
// rsense = 0.017 Ohm
#define POWER_MODULE_RSENSE 17

void signpost_energy_init ();

int signpost_energy_get_controller_energy ();
int signpost_energy_get_linux_energy ();
int signpost_energy_get_module_energy (int module_num);
void signpost_energy_reset ();
int signpost_ltc_to_uAh (int ltc_energy, int rsense, int prescaler);

