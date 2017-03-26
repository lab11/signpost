#include "i2c_selector.h"
#include "ltc2943.h"
#include "max17205.h"
#include "signpost_energy_ltc2943.h"

static uint8_t module_num_to_selector_mask[8] = {0x4, 0x8, 0x10, 0, 0, 0x20, 0x40, 0x80};

void signpost_energy_init (void) {
    // configure each ltc with the correct prescaler
    for (int i = 0; i < 8; i++) {
        i2c_selector_select_channels_sync(1<<i);
        ltc2943_configure_sync(InterruptPinAlertMode, POWER_MODULE_PRESCALER, ADCScan);
    }

    // set all channels open for Alert Response
    i2c_selector_select_channels_sync(0xFF);
}

static int get_ltc_energy (int selector_mask) {
	// Select correct LTC2943
	i2c_selector_select_channels_sync(selector_mask);

	// Get charge
	return ltc2943_get_charge_sync();
}

int signpost_energy_get_controller_energy (void) {
	return get_ltc_energy(0x1);
}

int signpost_energy_get_linux_energy (void) {
	return get_ltc_energy(0x2);
}

int signpost_energy_get_module_energy (int module_num) {
	return get_ltc_energy(module_num_to_selector_mask[module_num]);
}

int signpost_energy_get_battery_voltage (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    return voltage;
}

int signpost_energy_get_battery_current (void) {
    uint16_t voltage;
    int16_t current;
    max17205_read_voltage_current_sync(&voltage,&current);
    return current;
}

int signpost_energy_get_solar_voltage (void) {
    //selector #3 slot 1
    i2c_selector_select_channels_sync(0x100);

    return ltc2943_get_voltage_sync();
}

int signpost_energy_get_solar_current (void) {
    //selector #3 slot 1
    i2c_selector_select_channels_sync(0x100);

    return ltc2943_get_current_sync();
}

void signpost_energy_reset (void) {
	for (int i = 0; i < 8; i++) {
		i2c_selector_select_channels_sync(1<<i);

		ltc2943_reset_charge_sync();
	}
}

__attribute__((const))
int signpost_ltc_to_uAh (int ltc_energy, int rsense, int prescaler) {
    // prescaler needs to be a power of two
    /*uint8_t power = 0;
    uint8_t M;
    for(int i = 0; i < 8; i++) {
        if ((1<<i) & prescaler)
            power = i;
    }
    M = 1<<power;*/

    // Note that decimal values will be floored
    return ltc_energy * (((340*50)/rsense)*prescaler)/4096.0;
}

