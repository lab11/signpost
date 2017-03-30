#include "i2c_selector.h"
#include "ltc2941.h"
#include "ltc2943.h"
#include "signpost_energy.h"
#include "signpost_api.h"
#include "max17205.h"
#include "timer.h"

static uint8_t module_num_to_selector_mask[8] = {0x4, 0x8, 0x10, 0, 0, 0x20, 0x40, 0x80};
static uint8_t is_ltc2943 = 0;

/////////////////////////////////////////////////////
//These are the RAM variables that we update
//We are going to store them in nonvolatile memory too
/////////////////////////////////////////////////////
static int module_energy_remaining[8] = {0};
static int controller_energy_remaining = 0;

static int battery_last_energy_remaining = 0;

static int module_average_current[8] = {0};
static int controller_average_current = 0;
static int linux_average_current = 0;
static int battery_average_current = 0;
static int solar_average_current = 0;

static int module_energy[8] = {0};
static int controller_energy;
static int solar_energy;
static int linux_energy;
static int battery_energy_remaining;

static int module_energy_used[8] = {0};
static unsigned int last_time = 0;

#define BATTERY_CAPACITY 9000000
#define MAX_CONTROLLER_ENERGY_REMAINING BATTERY_CAPACITY*0.4
#define MAX_MODULE_ENERGY_REMAINING BATTERY_CAPACITY*0.1

////////////////////////////////////////////////////////////
//Initialization functions
//Make sure to call the right one
/////////////////////////////////////////////////////////
void signpost_energy_init (void) {
    // configure each ltc with the correct prescaler
    for (int i = 0; i < 8; i++) {
        i2c_selector_select_channels_sync(1<<i);
        ltc2941_configure_sync(InterruptPinAlertMode, POWER_MODULE_PRESCALER, VbatAlertOff);
    }

    is_ltc2943 = 0;
    // set all channels open for Alert Response
    i2c_selector_select_channels_sync(0xFF);
}

void signpost_energy_init_ltc2943 (signpost_energy_remaining_t* r) {
    // configure each ltc with the correct prescaler
    for (int i = 0; i < 9; i++) {
        i2c_selector_select_channels_sync(1<<i);
        ltc2943_configure_sync(InterruptPinAlertMode, POWER_MODULE_PRESCALER_LTC2943, ADCScan);
    }

    is_ltc2943 = 1;
    // set all channels open for Alert Response
    i2c_selector_select_channels_sync(0x1FF);

    if(r == NULL) {
        //initialize all of the energy remainings
        //...We really should do this in a nonvolatile way
        int battery_remaining = signpost_energy_get_battery_energy_remaining();
        controller_energy_remaining = battery_remaining*0.4;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 4 || i == 3) {

            } else {
                module_energy_remaining[i] = battery_remaining*0.1;
            }
        }
    } else {
        controller_energy_remaining = r->controller_energy_remaining;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 3 || i ==4) {

            } else {
                module_energy_remaining[i] = r->module_energy_remaining[i];
            }
        }
    }
    
    
    //reset all of the coulomb counters for the algorithm to work
    signpost_energy_reset_all_energy();

    //read the timer so the first iteration works out
    last_time = timer_read();
}

////////////////////////////////////////////////
// Static helper functions
// ////////////////////////////////////////////
static int get_ltc_energy (int selector_mask) {
	// Select correct LTC2941
	i2c_selector_select_channels_sync(selector_mask);

	// Get charge
	return ltc2941_get_charge_sync();
}

static int get_ltc_current_ua (int selector_mask) {
    i2c_selector_select_channels_sync(selector_mask);

    return ltc2943_convert_to_current_ua(ltc2943_get_current_sync(),17);
}

static void reset_ltc_energy (int selector_mask) {
    i2c_selector_select_channels_sync(selector_mask);
	ltc2941_reset_charge_sync();
}

static int ltc_to_uAh (int ltc_energy, int rsense) {
    if(!is_ltc2943) {
        return (int)(ltc_energy * (float)(85.0)*(50.0/rsense)*(POWER_MODULE_PRESCALER/128.0));
    } else {
        return (int)(ltc_energy * (float)(340.0)*(50.0/rsense)*(POWER_MODULE_PRESCALER_LTC2943/4096.0));
    }
}

///////////////////////////////////////////////////
// Energy Reset Functions
// ////////////////////////////////////////////////
void signpost_energy_reset_all_energy (void) {
    signpost_energy_reset_controller_energy();
    signpost_energy_reset_solar_energy();
    signpost_energy_reset_linux_energy();
    for(uint8_t i = 0; i < 8; i++) {
        signpost_energy_reset_module_energy(i);
    }
}

void signpost_energy_reset_controller_energy (void) {
    reset_ltc_energy(0x1);
}

void signpost_energy_reset_module_energy (int module_num) {
    if(module_num != 3 && module_num != 4) {
        reset_ltc_energy(module_num_to_selector_mask[module_num]);
    }
}

void signpost_energy_reset_solar_energy (void) {
    reset_ltc_energy(0x100);
}

void signpost_energy_reset_linux_energy (void) {
    reset_ltc_energy(0x2);
}

///////////////////////////////////////////////////
// Raw coulomb counter read functions
// ///////////////////////////////////////////////
int signpost_energy_get_controller_energy (void) {
	return ltc_to_uAh(get_ltc_energy(0x1),17);
}
int signpost_energy_get_linux_energy (void) {
	return ltc_to_uAh(get_ltc_energy(0x2),17);
}
int signpost_energy_get_module_energy (int module_num) {
	return ltc_to_uAh(get_ltc_energy(module_num_to_selector_mask[module_num]),17);
}
int signpost_energy_get_solar_energy (void) {
	return ltc_to_uAh(get_ltc_energy(0x100),50);
}

////////////////////////////////////////////////////
// Raw current read functions
// /////////////////////////////////////////////////////

int signpost_energy_get_controller_current (void) {
    return get_ltc_current_ua(0x1);
}

int signpost_energy_get_linux_current (void) {
    return get_ltc_current_ua(0x2);
}

int signpost_energy_get_module_current (int module_num) {
    return get_ltc_current_ua(module_num_to_selector_mask[module_num]);
}

int signpost_energy_get_solar_current (void) {
    return get_ltc_current_ua(0x100);
}

int signpost_energy_get_battery_current (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    float c = max17205_get_current_uA(current);
    return (int)c;
}

////////////////////////////////////////////////////////////////
// These functions tell you how much energy the module has remaining in it's "capacitor"
// This is updated at every call to the update function
// /////////////////////////////////////////////////////////////

int signpost_energy_get_controller_energy_remaining (void) {
    return controller_energy_remaining;
}

int signpost_energy_get_module_energy_remaining (int module_num) {
    return module_energy_remaining[module_num];
}

int signpost_energy_get_battery_energy_remaining (void) {
    uint16_t coulomb_volts;
    max17205_read_coulomb_sync(&coulomb_volts);
    float coulombs = coulomb_volts/0.01;
    return (int)coulombs;
}

////////////////////////////////////////////////////////////////////////////
// These functions tell you the average energy of this module over the last period
// This is updated at every call to the update function
// /////////////////////////////////////////////////////////////////////////

int signpost_energy_get_controller_average_current (void) {
    return controller_average_current;
}

int signpost_energy_get_linux_average_current(void) {
    return linux_average_current;
}

int signpost_energy_get_module_average_current (int module_num) {
    return module_average_current[module_num];
}

int signpost_energy_get_battery_average_current (void) {
    return battery_average_current;
}

int signpost_energy_get_solar_average_current (void) {
    return solar_average_current;
}


/////////////////////////////////////////////////////////////////////
// These functions give you voltages for battery and solar
// ///////////////////////////////////////////////////////////////////
int signpost_energy_get_battery_voltage_mv (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    float v = max17205_get_voltage_mV(voltage);
    return (int)v;
}



int signpost_energy_get_solar_voltage_mv (void) {
    //selector #3 slot 1
    i2c_selector_select_channels_sync(0x100);

    return ltc2943_convert_to_voltage_mv(ltc2943_get_voltage_sync());
}



///////////////////////////////////////////////////////////////////
//The big update function
///////////////////////////////////////////////////////////////
void signpost_energy_update_energy (void) {

    //first let's look at how long it has been since this was called
    unsigned int time_now = timer_read();
    unsigned int time;
    if(time_now < last_time) {
        time = (unsigned int)(((0xFFFFFFFF - last_time) + time_now)/16000.0);
    } else {
        time = (unsigned int)((last_time-time_now)/16000.0);
    }
    last_time = time_now;

    static int total_energy_used = 0;
    //now let's read all the coulomb counters
    linux_energy = signpost_energy_get_linux_energy();
    total_energy_used += linux_energy;
    controller_energy = signpost_energy_get_controller_energy();
    total_energy_used += controller_energy;
    solar_energy = signpost_energy_get_solar_energy();
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            module_energy[i] = signpost_energy_get_module_energy(i);
            total_energy_used += module_energy[i];
            module_energy_used[i] += module_energy[i];
        }
    }
    battery_energy_remaining = signpost_energy_get_battery_energy_remaining();

    //reset all of the coulomb counters so we can use them next time
    signpost_energy_reset_all_energy();

    //update all the averages over the amount of time used
    linux_average_current = (linux_energy*3600)/time;
    controller_average_current = (controller_energy*3600)/time;
    solar_average_current = (solar_energy*3600)/time;
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            module_average_current[i] = (module_energy[i]*3600)/time;
        }
    }
    battery_average_current = ((battery_last_energy_remaining-battery_energy_remaining)*3600)/time;


    //Now we should subtract all of the energies from what the modules had before
    controller_energy_remaining -= controller_energy;
    controller_energy_remaining -= linux_energy;
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            module_energy_remaining[i] -= module_energy[i];
        }
    }

    //now we need to figure out how much energy (if any) we got
    //This needs to be distributed among the modules
    // technically battery_energy_remaining = battery_last_energy_remaining - total_energy_used + solar_energy
    // This isn't going to be true due to efficiency losses and such
    // But what we can do:
    if(battery_energy_remaining > battery_last_energy_remaining - total_energy_used) {
        //we have surplus!! let's distribute it
        int surplus = battery_energy_remaining - (battery_last_energy_remaining - total_energy_used);
        int controller_surplus = (int)(surplus * 0.4);
        int module_surplus = (int)(surplus * 0.1);
        
        controller_energy_remaining += controller_surplus;
        if(controller_energy_remaining > MAX_CONTROLLER_ENERGY_REMAINING) {
            module_surplus += (int)((controller_energy_remaining - MAX_CONTROLLER_ENERGY_REMAINING)/6.0);
        }

        //this is a two pass algorithm which can be games. Really it would take n passes to do it right
        //I don't want to code the npass algorithm really, when are all the
        //modules going to be full anyways?
        int spill_over = 0;
        uint8_t spill_elgible_count = 0;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 3 || i == 4) {

            } else {
                if(module_energy_remaining[i] + module_surplus > MAX_MODULE_ENERGY_REMAINING) {
                    spill_over += (module_energy_remaining[i] + module_surplus) - MAX_MODULE_ENERGY_REMAINING;
                    module_energy_remaining[i] = MAX_MODULE_ENERGY_REMAINING;
                } else {
                    module_energy_remaining[i] += module_surplus;
                    spill_elgible_count++;
                }
            }
        }

        spill_over = spill_over/spill_elgible_count;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 3 || i == 4) {

            } else {
                if(module_energy_remaining[i] >= MAX_MODULE_ENERGY_REMAINING) {

                } else {
                    module_energy_remaining[i] += spill_over;
                }
            }
        }
    } else {
        //efficiency losses - we should probably also distribute those losses (or charge them to the controller?)
        controller_energy_remaining -= ((battery_last_energy_remaining - total_energy_used) - battery_energy_remaining);
    }

    battery_last_energy_remaining = battery_energy_remaining; 
}

void signpost_energy_update_energy_from_report(uint8_t source_module_slot, signpost_energy_report_t* report) {
    int energy = module_energy_used[source_module_slot];
    int other_energy = 0;
    uint8_t num_reports = report->num_reports;
    for(uint8_t j = 0; j < num_reports; j++) {
        module_energy_remaining[report->reports[j].module_address] -= (int)(energy*(report->reports[j].module_percent/100.0));
        module_energy_used[report->reports[j].module_address] += (int)(energy*report->reports[j].module_percent/100.0);
        other_energy += (int)(energy*report->reports[j].module_percent/100.0);
    }
    module_energy_remaining[source_module_slot] += other_energy;
    module_energy_used[source_module_slot] = 0;
}
