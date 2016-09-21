#include <firestorm.h>
#include <tock.h>
#include <i2c_selector.h>

#define DRIVER_NUM_I2C_SELECTOR 101


int i2c_selector_set_callback(subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_GPIO_ASYNC, 0, callback, callback_args);
}

int i2c_selector_select_channels(uint32_t channels) {
	return command(DRIVER_NUM_GPIO_ASYNC, 0, channels);
}

int i2c_selector_select_channels() {
	return command(DRIVER_NUM_GPIO_ASYNC, 1, 0);
}
