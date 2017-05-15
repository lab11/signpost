#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "signpost_api.h"

typedef struct energy_remaining {
    int controller_energy_remaining;
    int module_energy_remaining[8];
} signpost_energy_remaining_t;

typedef struct average_power {
    int controller_average_power;
    int module_average_power[8];
} signpost_average_power_t;


//if r == NULL then initialize from battery capacity
void signpost_energy_policy_init(signpost_energy_remaining_t* r, signpost_average_power_t* p);

//these functions tell you how much each module has remaining in their logical capacitors
//this is returned to the module on an energy query
int signpost_energy_policy_get_controller_energy_remaining (void);
int signpost_energy_policy_get_module_energy_remaining (int module_num);
int signpost_energy_policy_get_battery_energy_remaining (void);

//these functions tell you the average current for each module over the last update period
//this is what is returned to the module on an energy query
int signpost_energy_policy_get_controller_average_power (void);
int signpost_energy_policy_get_linux_average_power (void);
int signpost_energy_policy_get_module_average_power (int module_num);
int signpost_energy_policy_get_battery_average_power (void);
int signpost_energy_policy_get_solar_average_power (void);

//this function should be called every 600s to update energy capacities
void signpost_energy_policy_update_energy (void);

//this function should be called to distribute the energy of one module
//to other modules - primarily for the radio right now
void signpost_energy_policy_update_energy_from_report(uint8_t source_module_slot, signpost_energy_report_t* report);

#ifdef __cplusplus
}
#endif
