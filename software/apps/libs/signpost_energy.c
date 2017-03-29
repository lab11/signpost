#include "i2c_selector.h"
#include "ltc2941.h"
#include "ltc2943.h"
#include "signpost_energy.h"
#include "max17205.h"

static uint8_t module_num_to_selector_mask[8] = {0x4, 0x8, 0x10, 0, 0, 0x20, 0x40, 0x80};
static uint8_t is_ltc2943 = 0;

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

void signpost_energy_init_ltc2943 (void) {
    // configure each ltc with the correct prescaler
    for (int i = 0; i < 9; i++) {
        i2c_selector_select_channels_sync(1<<i);
        ltc2943_configure_sync(InterruptPinAlertMode, POWER_MODULE_PRESCALER_LTC2943, ADCScan);
    }

    is_ltc2943 = 1;
    // set all channels open for Alert Response
    i2c_selector_select_channels_sync(0x1FF);

    //max17205_configure_pack_sync();
}

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

int signpost_energy_get_controller_energy (void) {
	return get_ltc_energy(0x1);
}

int signpost_energy_get_controller_current_ua (void) {
    return get_ltc_current_ua(0x1);
}

int signpost_energy_get_linux_energy (void) {
	return get_ltc_energy(0x2);
}

int signpost_energy_get_linux_current_ua (void) {
    return get_ltc_current_ua(0x2);
}

int signpost_energy_get_module_energy (int module_num) {
	return get_ltc_energy(module_num_to_selector_mask[module_num]);
}

int signpost_energy_get_module_current_ua (int module_num) {
    return get_ltc_current_ua(module_num_to_selector_mask[module_num]);
}

int signpost_energy_get_battery_voltage_mv (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    float v = max17205_get_voltage_mV(voltage);
    return (int)v;
}

int signpost_energy_get_battery_current_ua (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    float c = max17205_get_current_uA(current);
    return (int)c;
}

int signpost_energy_get_battery_energy (void) {
    uint16_t percent;
    uint16_t charge;
    uint16_t full;
    max17205_read_soc_sync(&percent, &charge, &full);
    return max17205_get_capacity_uAh(charge);
}

int signpost_energy_get_battery_percent (void) {
    uint16_t percent;
    uint16_t charge;
    uint16_t full;
    max17205_read_soc_sync(&percent, &charge, &full);
    return max17205_get_percentage_mP(percent);
}

int signpost_energy_get_battery_capacity (void) {
    uint16_t percent;
    uint16_t charge;
    uint16_t full;
    max17205_read_soc_sync(&percent, &charge, &full);
    return max17205_get_capacity_uAh(full);
}

int signpost_energy_get_solar_voltage_mv (void) {
    //selector #3 slot 1
    i2c_selector_select_channels_sync(0x100);

    return ltc2943_convert_to_voltage_mv(ltc2943_get_voltage_sync());
}

int signpost_energy_get_solar_current_ua (void) {
    //selector #3 slot 1
    i2c_selector_select_channels_sync(0x100);

    return ltc2943_convert_to_current_ua(ltc2943_get_current_sync(),50);
}

void signpost_energy_reset (void) {
	for (int i = 0; i < 8; i++) {
		i2c_selector_select_channels_sync(1<<i);

		ltc2941_reset_charge_sync();
	}

    if(is_ltc2943 ==1) {
		i2c_selector_select_channels_sync(0x100);

		ltc2941_reset_charge_sync();
    }
}

__attribute__((const))
int signpost_ltc_to_uAh (int ltc_energy, int rsense, int prescaler) {
    if(!is_ltc2943) {
        // prescaler needs to be a power of two
        uint8_t power = 0;
        uint8_t M;
        for(int i = 0; i < 8; i++) {
            if ((1<<i) & prescaler)
                power = i;
        }
        M = 1<<power;
        // Note that decimal values will be floored
        return ltc_energy * (((85*50)/rsense)*M)/128;

    } else {
        return (int)(ltc_energy * (float)(340.0)*(50.0/rsense)*(prescaler/4096.0));
    }
}

