#include "signpost_energy_policy.h"
#include "signpost_energy_monitors.h"
#include "signpost_api.h"
#include "timer.h"
#include <stdio.h>

/////////////////////////////////////////////////////
//These are the RAM variables that we update
//We are going to store them in nonvolatile memory too
/////////////////////////////////////////////////////
static int controller_energy_remaining = 0;

static int battery_last_energy_remaining = 0;

static int controller_average_power = 0;
static int linux_average_power = 0;
static int battery_average_power = 0;
static int solar_average_power = 0;

static int controller_energy_used_since_update = 0;
static int linux_energy;
static int battery_energy_remaining;

static int total_energy_used_since_update = 0;
static int module_average_power[8] = {0};
static int module_energy_remaining[8] = {0};
static int module_energy_used_since_update[8] = {0};
static int module_energy_used_since_report[8] = {0};
static unsigned int last_time = 0;

#define BATTERY_CAPACITY 9000000*11.1
#define MAX_CONTROLLER_ENERGY_REMAINING BATTERY_CAPACITY*0.4
#define MAX_MODULE_ENERGY_REMAINING BATTERY_CAPACITY*0.1

void signpost_energy_policy_init (signpost_energy_remaining_t* r, signpost_average_power_t* p) {


    if(r == NULL) {
        //initialize all of the energy remainings
        //...We really should do this in a nonvolatile way
        int battery_remaining = signpost_energy_get_battery_energy();
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

    if(p == NULL) {
        //initialize all of the energy remainings
        controller_average_power = 0;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 4 || i == 3) {

            } else {
                module_average_power[i] = 0;
            }
        }
    } else {
        controller_average_power = p->controller_average_power;
        for(uint8_t i = 0; i < 8; i++) {
            if(i == 3 || i ==4) {

            } else {
                module_average_power[i] = p->module_average_power[i];
            }
        }
    }


    //reset all of the coulomb counters for the algorithm to work
    signpost_energy_reset_all_energy();

    //read the timer so the first iteration works out
    last_time = timer_read();

    //read the battery now so that the first interation works
    int bat = signpost_energy_get_battery_energy();
    battery_last_energy_remaining = bat;
}

////////////////////////////////////////////////////////////////
// These functions tell you how much energy the module has remaining in it's "capacitor"
// This is updated at every call to the update function
// /////////////////////////////////////////////////////////////

int signpost_energy_policy_get_controller_energy_remaining (void) {
    return controller_energy_remaining;
}

int signpost_energy_policy_get_module_energy_remaining (int module_num) {
    return module_energy_remaining[module_num];
}

////////////////////////////////////////////////////////////////////////////
// These functions tell you the average energy of this module over the last period
// This is updated at every call to the update function
// /////////////////////////////////////////////////////////////////////////

int signpost_energy_policy_get_controller_average_power (void) {
    return controller_average_power;
}

int signpost_energy_policy_get_linux_average_power(void) {
    return linux_average_power;
}

int signpost_energy_policy_get_module_average_power (int module_num) {
    return module_average_power[module_num];
}

int signpost_energy_policy_get_battery_average_power (void) {
    return battery_average_power;
}

int signpost_energy_policy_get_solar_average_power (void) {
    return solar_average_power;
}


///////////////////////////////////////////////////////////////////
//The big update function
///////////////////////////////////////////////////////////////
void signpost_energy_policy_update_energy (void) {

    //first let's look at how long it has been since this was called
    unsigned int time_now = timer_read();
    unsigned int time;
    if(time_now < last_time) {
        time = (unsigned int)(((0xFFFFFFFF - last_time) + time_now)/16000.0);
    } else {
        time = (unsigned int)((time_now-last_time)/16000.0);
    }
    last_time = time_now;

    //now let's read all the coulomb counters
    linux_energy = signpost_energy_get_linux_energy();
    printf("ENERGY: Linux used %d uWh since last update\n",signpost_energy_get_linux_energy());
    total_energy_used_since_update += linux_energy;
    controller_energy_used_since_update += signpost_energy_get_controller_energy();
    printf("ENERGY: Controller used %d uWh since last update\n", controller_energy_used_since_update);
    total_energy_used_since_update += controller_energy_used_since_update;
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            module_energy_used_since_update[i] += signpost_energy_get_module_energy(i);
            module_energy_used_since_report[i] += signpost_energy_get_module_energy(i);
            total_energy_used_since_update += module_energy_used_since_update[i];
            printf("ENERGY: Module %d used %d uWh since last update\n",i,module_energy_used_since_update[i]);
        }
    }
    battery_energy_remaining = signpost_energy_get_battery_energy();
    printf("ENERGY: Battery has %d uWh remaining\n",signpost_energy_get_battery_energy());



    //reset all of the coulomb counters so we can use them next time
    signpost_energy_reset_all_energy();

    //update all the averages over the amount of time used
    linux_average_power = (int)(linux_energy*3600)/time;
    controller_average_power = (int)(controller_energy_used_since_update*3600)/time;
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            if(module_energy_used_since_update[i] < 0) {
                module_average_power[i] = 0;
            } else {
                module_average_power[i] = (module_energy_used_since_update[i]*3600)/time;
            }
        }
    }

    battery_average_power = ((battery_last_energy_remaining-battery_energy_remaining)*3600)/time;


    //Now we should subtract all of the energies from what the modules had before
    controller_energy_remaining -= controller_energy_used_since_update;
    controller_energy_remaining -= linux_energy;
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) {

        } else {
            module_energy_remaining[i] -= module_energy_used_since_update[i];
        }
        module_energy_used_since_update[i] = 0;
    }

    controller_energy_used_since_update = 0;
    linux_energy = 0;

    printf("ENERGY: Total energy since update: %d uWh\n", total_energy_used_since_update);

    //now we need to figure out how much energy (if any) we got
    //This needs to be distributed among the modules
    // technically battery_energy_remaining = battery_last_energy_remaining - total_energy_used + solar_energy
    // This isn't going to be true due to efficiency losses and such
    // But what we can do:
    if(battery_energy_remaining > battery_last_energy_remaining - total_energy_used_since_update) {
        //we have surplus!! let's distribute it
        int surplus = battery_energy_remaining - (battery_last_energy_remaining - total_energy_used_since_update);
        int controller_surplus = (int)(surplus * 0.4);
        int module_surplus = (int)(surplus * 0.1);

        controller_energy_remaining += controller_surplus;
        if(controller_energy_remaining > MAX_CONTROLLER_ENERGY_REMAINING) {
            module_surplus += (int)((controller_energy_remaining - MAX_CONTROLLER_ENERGY_REMAINING)/6.0);
            controller_energy_remaining = MAX_CONTROLLER_ENERGY_REMAINING;
        }

        //this is a two pass algorithm which can be games. Really it would take n passes to do it right
        //I don't want to code the npass algorithm really, when are all the
        //modules going to be full anyways?
        uint8_t spill_elgible[8] = {1};
        while(module_surplus > 0) {
            int spill_over = 0;
            uint8_t spill_elgible_count = 0;

            //try to distribute the energy
            for(uint8_t i = 0; i < 8; i++) {
                if(i == 3 || i == 4 || spill_elgible[i] == 0)  continue;

                if(module_energy_remaining[i] + module_surplus > MAX_MODULE_ENERGY_REMAINING) {
                    spill_over += (module_energy_remaining[i] + module_surplus) - MAX_MODULE_ENERGY_REMAINING;
                    module_energy_remaining[i] = MAX_MODULE_ENERGY_REMAINING;
                    spill_elgible[i] = 0;
                } else {
                    module_energy_remaining[i] += module_surplus;
                    spill_elgible_count++;
                    spill_elgible[i] = 1;
                }
            }

            //if everything is full give to controller, else distribute again
            if(spill_elgible_count == 0) {
                module_surplus = 0;
                controller_energy_remaining += spill_over;
            } else {
                module_surplus = spill_over/spill_elgible_count;
            }
        }

    } else {
        //efficiency losses - we should probably also distribute those losses (or charge them to the controller?)
        controller_energy_remaining -= ((battery_last_energy_remaining - total_energy_used_since_update) - battery_energy_remaining);
    }

    total_energy_used_since_update = 0;
    battery_last_energy_remaining = battery_energy_remaining;
}

void signpost_energy_policy_update_energy_from_report(uint8_t source_module_slot, signpost_energy_report_t* report) {
    //read the source module's energy
    int used_since_update = signpost_energy_get_module_energy(source_module_slot);

    //add it to that used since update
    total_energy_used_since_update += used_since_update;

    //reset that slot
    signpost_energy_reset_module_energy(source_module_slot);

    //add the energy to the total energy used since last report
    module_energy_used_since_report[source_module_slot] += used_since_update;

    //also add it to the total energy used by the module since the last update
    module_energy_used_since_update[source_module_slot] += used_since_update;

    //copy to local variable because it's easier
    int energy = module_energy_used_since_report[source_module_slot];

    uint8_t num_reports = report->num_reports;
    for(uint8_t j = 0; j < num_reports; j++) {
        //take the energy since last report, add/subtract it from all the totals
        if(report->reports[j].module_address == 3) {
            controller_energy_used_since_update += (int)(energy*report->reports[j].module_percent/100.0);

            module_energy_used_since_update[source_module_slot] -= (int)(energy*(report->reports[j].module_percent/100.0));
        } else {
            module_energy_used_since_report[report->reports[j].module_address] += (int)(energy*report->reports[j].module_percent/100.0);
            module_energy_used_since_update[report->reports[j].module_address] += (int)(energy*report->reports[j].module_percent/100.0);

            module_energy_used_since_update[source_module_slot] -= (int)(energy*(report->reports[j].module_percent/100.0));

        }
    }

    module_energy_used_since_report[source_module_slot] = 0;
}
