#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// qlsb = 0.0625mAh with 0.017 Ohm sense resistor
#define POWER_MODULE_PRESCALER 32
#define POWER_MODULE_PRESCALER_LTC2943 64

//in Mohm
#define POWER_MODULE_RSENSE 17
#define POWER_MODULE_SOLAR_RSENSE 50

//voltages for the energy numbers
#define MODULE_VOLTAGE 5
#define LINUX_VOLTAGE 5
#define CONTROLLER_VOLTAGE 3.3
#define BATTERY_VOLTAGE_NOM 11.1


void signpost_energy_init (void);

//initialize the remaining values
void signpost_energy_init_ltc2943 (void);

//this zeros the coulomb counters for each counter
void signpost_energy_reset_all_energy (void);
void signpost_energy_reset_controller_energy (void);
void signpost_energy_reset_linux_energy (void);
void signpost_energy_reset_solar_energy (void);
void signpost_energy_reset_module_energy (int module_num);

//these functions return the energy used in uWh
int signpost_energy_get_controller_energy (void);
int signpost_energy_get_linux_energy (void);
int signpost_energy_get_module_energy (int module_num);

//these function return return the instantaneous current in uA
int signpost_energy_get_battery_current (void);
int signpost_energy_get_solar_current (void);
int signpost_energy_get_controller_current (void);
int signpost_energy_get_linux_current (void);
int signpost_energy_get_module_current (int module_num);

//some other battery support functions

//uWh
int signpost_energy_get_battery_capacity (void);

//whole number percent
int signpost_energy_get_battery_percent (void);

//uWh
int signpost_energy_get_battery_energy (void);

//these functions tell you instantaneous current and voltage for the battery and solar panel
int signpost_energy_get_battery_voltage (void);
int signpost_energy_get_solar_voltage (void);

#ifdef __cplusplus
}
#endif
