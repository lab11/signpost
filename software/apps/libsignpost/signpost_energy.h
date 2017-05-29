#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// qlsb = 0.0625mAh with 0.017 Ohm sense resistor
#define POWER_MODULE_PRESCALER_LTC2941 32
#define POWER_MODULE_PRESCALER_LTC2943 256
// rsense = 0.017 Ohm
#define POWER_MODULE_RSENSE 17

void signpost_energy_init (void);
void signpost_energy_init_ltc2943 (void);

int signpost_energy_get_controller_energy (void);
int signpost_energy_get_controller_current_ua (void);
int signpost_energy_get_linux_energy (void);
int signpost_energy_get_linux_current_ua (void);
int signpost_energy_get_module_energy (int module_num);
int signpost_energy_get_module_current_ua (int module_num);
int signpost_energy_get_battery_voltage_mv (void);
int signpost_energy_get_battery_current_ua (void);
int signpost_energy_get_battery_energy (void);
int signpost_energy_get_battery_percent (void);
int signpost_energy_get_battery_capacity (void);
int signpost_energy_get_solar_voltage_mv (void);
int signpost_energy_get_solar_current_ua (void);
void signpost_energy_reset (void);

int signpost_ltc_to_uAh (int ltc_energy, int rsense);

#ifdef __cplusplus
}
#endif
