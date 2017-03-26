#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// qlsb = 0.0625mAh with 0.017 Ohm sense resistor
#define POWER_MODULE_PRESCALER 256
// rsense = 0.017 Ohm
#define POWER_MODULE_RSENSE 17

void signpost_energy_init (void);

int signpost_energy_get_controller_energy (void);
int signpost_energy_get_linux_energy (void);
int signpost_energy_get_module_energy (int module_num);
int signpost_energy_get_battery_voltage (void);
int signpost_energy_get_battery_current (void);
int signpost_energy_get_solar_voltage (void);
int signpost_energy_get_solar_current (void);
void signpost_energy_reset (void);
int signpost_ltc_to_uAh (int ltc_energy, int rsense, int prescaler);

#ifdef __cplusplus
}
#endif
