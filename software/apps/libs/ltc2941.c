#include "tock.h"
#include "ltc2941.h"


struct ltc2941_data {
  int charge;
  bool fired;
};

static struct ltc2941_data result = { .fired = false, .charge = 0 };

// Internal callback for faking synchronous reads
static void ltc2941_cb(__attribute__ ((unused)) int callback_type,
                       __attribute__ ((unused)) int value,
                       __attribute__ ((unused)) int chip,
                       void* ud) {
  struct ltc2941_data* result = (struct ltc2941_data*) ud;
  result->charge = value;
  result->fired = true;
}

int ltc2941_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_LTC2941, 0, callback, callback_args);
}

int ltc2941_read_status() {
    return command(DRIVER_NUM_LTC2941, 0, 0);
}

int ltc2941_configure(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat) {
    uint8_t M = 0;
        // ltc2941 expects log_2 of prescaler value
        for(int i = 0; i < 8; i++) {
            if ((1<<i) & prescaler)
                M = i;
        }
        uint8_t cmd = (int_pin & 0x03) | ((M & 0x07) << 2) | ((vbat & 0x03) << 5);
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



int ltc2941_read_status_sync() {
    int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_read_status();
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2941_configure_sync(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat) {
	int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_configure(int_pin, prescaler, vbat);
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2941_reset_charge_sync() {
    int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_reset_charge();
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2941_set_high_threshold_sync(uint16_t threshold) {
	int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_set_high_threshold(threshold);
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2941_set_low_threshold_sync(uint16_t threshold) {
	int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_set_low_threshold(threshold);
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2941_get_charge_sync() {
	int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_get_charge();
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return result.charge;
}

int ltc2941_shutdown_sync() {
	int err;
    result.fired = false;

    err = ltc2941_set_callback(ltc2941_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2941_shutdown();
    if (err < 0) return err;

    // Wait for the ADC callback.
    yield_for(&result.fired);

    return 0;
}
