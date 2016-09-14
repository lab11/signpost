#include <firestorm.h>
#include <tock.h>
#include <ltc2941.h>

#define DRIVER_NUM_LTC2941 102


int ltc2941_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_LTC2941, 0, callback, callback_args);
}

int ltc2941_read_status() {
	return command(DRIVER_NUM_LTC2941, 0, 0);
}

int ltc2941_configure(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat) {
	uint8_t cmd = (int_pin & 0x03) | ((prescaler & 0x07) << 2) | ((vbat & 0x03) << 5);
	return command(DRIVER_NUM_LTC2941, 1, cmd);
}

int ltc2941_reset_charge() {
	return command(DRIVER_NUM_LTC2941, 2, 0);
}

int ltc2941_set_high_threshold(uint16_t threshold) {
	return command(DRIVER_NUM_LTC2941, 3, threshold);
}

int ltc2941_set_low_threshold(uint16_t threshold) {
	return command(DRIVER_NUM_LTC2941, 4, threshold);
}

int ltc2941_get_charge() {
	return command(DRIVER_NUM_LTC2941, 5, 0);
}

int ltc2941_shutdown() {
	return command(DRIVER_NUM_LTC2941, 6, 0);
}
