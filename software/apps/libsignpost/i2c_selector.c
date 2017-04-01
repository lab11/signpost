
#include "tock.h"
#include "i2c_selector.h"

#define DRIVER_NUM_I2C_SELECTOR 101


struct i2c_selector_data {
  bool fired;
  int value;
};

static struct i2c_selector_data result = { .fired = false };

// Internal callback for faking synchronous reads
static void i2c_selector_cb(__attribute__ ((unused)) int value,
                       __attribute__ ((unused)) int unused1,
                       __attribute__ ((unused)) int unused2,
                       void* ud) {
  struct i2c_selector_data* data = (struct i2c_selector_data*) ud;
  data->value = value;
  data->fired = true;
}


int i2c_selector_set_callback(subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_I2C_SELECTOR, 0, callback, callback_args);
}

int i2c_selector_select_channels(uint32_t channels) {
	return command(DRIVER_NUM_I2C_SELECTOR, 0, channels);
}

int i2c_selector_disable_all_channels(void) {
	return command(DRIVER_NUM_I2C_SELECTOR, 1, 0);
}

int i2c_selector_read_interrupts(void) {
	return command(DRIVER_NUM_I2C_SELECTOR, 2, 0);
}

int i2c_selector_read_selected(void) {
	return command(DRIVER_NUM_I2C_SELECTOR, 3, 0);
}



int i2c_selector_select_channels_sync(uint32_t channels) {
    int err;
    result.fired = false;

    err = i2c_selector_set_callback(i2c_selector_cb, (void*) &result);
    if (err < 0) return err;

    err = i2c_selector_select_channels(channels);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int i2c_selector_disable_all_channels_sync(void) {
    int err;
    result.fired = false;

    err = i2c_selector_set_callback(i2c_selector_cb, (void*) &result);
    if (err < 0) return err;

    err = i2c_selector_disable_all_channels();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int i2c_selector_read_interrupts_sync(void) {
    int err;
    result.fired = false;

    err = i2c_selector_set_callback(i2c_selector_cb, (void*) &result);
    if (err < 0) return err;

    err = i2c_selector_read_interrupts();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int i2c_selector_read_selected_sync(void) {
    int err;
    result.fired = false;

    err = i2c_selector_set_callback(i2c_selector_cb, (void*) &result);
    if (err < 0) return err;

    err = i2c_selector_read_selected();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}
