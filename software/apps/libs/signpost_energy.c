
#include "firestorm.h"

#include "signpost_energy.h"
#include "i2c_selector.h"
#include "ltc2941.h"


static uint8_t module_num_to_selector_mask[8] = {0x4, 0x8, 0x10, 0, 0, 0x20, 0x40, 0x80};

static int charge;

#define UNUSED_PARAMETER(x) (void)(x)




void i2c_selector_callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_type);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(data2);
  UNUSED_PARAMETER(callback_args);
}

void ltc2941_callback (int callback_type, int data, int data2, void* callback_args) {
  UNUSED_PARAMETER(callback_args);
  UNUSED_PARAMETER(data2);

  if (callback_type == 2) {
  	charge = data;
  }
}

void signpost_energy_init () {
	i2c_selector_set_callback(i2c_selector_callback, NULL);
	ltc2941_set_callback(ltc2941_callback, NULL);

    // configure each ltc with the correct prescaler
    for (int i = 0; i < 8; i++) {
        i2c_selector_select_channels(1<<i);
        yield();

        ltc2941_configure(InterruptPinAlertMode, POWER_MODULE_PRESCALER, VbatAlertOff);
        yield();
    }

    // set all channels open for Alert Response
    i2c_selector_select_channels(0xFF);
    yield();
}

static int get_ltc_energy (int selector_mask) {
	// delay_ms(200);

	// Select correct LTC2941
	i2c_selector_select_channels(selector_mask);
	yield();



	//i2c_selector_read_selected();
	//yield();

	// Get charge
	ltc2941_get_charge();
	yield();

	return charge;
}

int signpost_energy_get_controller_energy () {
	return get_ltc_energy(0x1);
}

int signpost_energy_get_linux_energy () {
	return get_ltc_energy(0x2);
}

int signpost_energy_get_module_energy (int module_num) {
	return get_ltc_energy(module_num_to_selector_mask[module_num]);
}

int signpost_energy_reset () {
	for (int i = 0; i < 8; i++) {
		i2c_selector_select_channels(1<<i);
		yield();

		ltc2941_reset_charge();
		yield();
	}
}

int signpost_ltc_to_uAh (int ltc_energy, int rsense, int prescaler) {
    // prescaler needs to be a power of two
    uint8_t power;
    uint8_t M;
    for(int i = 0; i < 8; i++) {
        if ((1<<i) & prescaler)
            power = i;
    }
    M = 1<<power;
    // Note that decimal values will be floored
    return ltc_energy * (((85*50)/rsense)*M)/128;
}

