#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "signpost_api.h"

// qlsb = 0.0625mAh with 0.017 Ohm sense resistor
#define POWER_MODULE_PRESCALER 32
#define POWER_MODULE_PRESCALER_LTC2943 64
// rsense = 0.017 Ohm
#define POWER_MODULE_RSENSE 17

typedef struct energy_remaining {
    int controller_energy_remaining;
    int module_energy_remaining[8];
} signpost_energy_remaining_t;

void signpost_energy_init (void);

//initialize the remaining values
//if r == NULL then initialize from battery capacity
void signpost_energy_init_ltc2943 (signpost_energy_remaining_t* r);

//this zeros the coulomb counters for each counter
void signpost_energy_reset_all_energy (void);
void signpost_energy_reset_controller_energy (void);
void signpost_energy_reset_linux_energy (void);
void signpost_energy_reset_solar_energy (void);
void signpost_energy_reset_module_energy (int module_num);

//these functions just return the raw value of the coulomb counters in uAh
int signpost_energy_get_controller_energy (void);
int signpost_energy_get_solar_energy (void);
int signpost_energy_get_linux_energy (void);
int signpost_energy_get_module_energy (int module_num);

//these function return return the instantaneous current in uA
int signpost_energy_get_battery_current (void);
int signpost_energy_get_solar_current (void);
int signpost_energy_get_controller_current (void);
int signpost_energy_get_linux_current (void);
int signpost_energy_get_module_current (int module_num);

//these functions tell you how much each module has remaining in their logical capacitors
//this is returned to the module on an energy query
int signpost_energy_get_controller_energy_remaining (void);
int signpost_energy_get_module_energy_remaining (int module_num);
int signpost_energy_get_battery_energy_remaining (void);

//these functions tell you the average current for each module over the last update period
//this is what is returned to the module on an energy query
int signpost_energy_get_controller_average_power (void);
int signpost_energy_get_linux_average_power (void);
int signpost_energy_get_module_average_power (int module_num);
int signpost_energy_get_battery_average_power (void);
int signpost_energy_get_solar_average_power (void);

//some other battery support functions
int signpost_energy_get_battery_capacity (void);
int signpost_energy_get_battery_percent (void);

//these functions tell you instantaneous current and voltage for the battery and solar panel
int signpost_energy_get_battery_voltage (void);
int signpost_energy_get_solar_voltage (void);

//this function should be called every 600s to update energy capacities
void signpost_energy_update_energy (void);

//this function should be called to distribute the energy of one module
//to other modules - primarily for the radio right now
void signpost_energy_update_energy_from_report(uint8_t source_module_slot, signpost_energy_report_t* report);

#ifdef __cplusplus
}
#endif
