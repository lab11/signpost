#include "tock.h"
#include "ltc2943.h"


struct ltc2943_data {
  int charge;
  bool fired;
};

static struct ltc2943_data result = { .fired = false, .charge = 0 };

// Internal callback for faking synchronous reads
static void ltc2943_cb(__attribute__ ((unused)) int callback_type,
                       __attribute__ ((unused)) int value,
                       __attribute__ ((unused)) int chip,
                       void* ud) {
  struct ltc2943_data* data = (struct ltc2943_data*) ud;
  data->charge = value;
  data->fired = true;
}

int ltc2943_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_LTC2941, 0, callback, callback_args);
}

int ltc2943_read_status(void) {
    return command(DRIVER_NUM_LTC2941, 0, 0);
}

int ltc2943_configure(interrupt_pin_conf_e int_pin, uint16_t prescaler, adc_mode_e adc) {
        uint8_t M = 0;
        switch(prescaler) {
            case 1:
                M = 0;
            break;
            case 4:
                M = 1;
            break;
            case 16:
                M = 2;
            break;
            case 64:
                M = 3;
            break;
            case 256:
                M = 4;
            break;
            case 1024:
                M = 5;
            break;
            case 4096:
                M = 7;
            break;
            default:
                M = 4;
            break;
        }
        uint8_t cmd = (int_pin & 0x03) | ((M & 0x07) << 2) | ((adc & 0x03) << 5);
        return command(DRIVER_NUM_LTC2941, 1, cmd);
}

int ltc2943_reset_charge(void) {
    return command(DRIVER_NUM_LTC2941, 2, 0);
}

int ltc2943_set_high_threshold(uint16_t threshold) {
    return command(DRIVER_NUM_LTC2941, 3, threshold);
}

int ltc2943_set_low_threshold(uint16_t threshold) {
    return command(DRIVER_NUM_LTC2941, 4, threshold);
}

int ltc2943_get_charge(void) {
    return command(DRIVER_NUM_LTC2941, 5, 0);
}

int ltc2943_get_voltage(void) {
    return command(DRIVER_NUM_LTC2941, 7, 0);
}

int ltc2943_get_current(void) {
    return command(DRIVER_NUM_LTC2941, 8, 0);
}

int ltc2943_shutdown(void) {
    return command(DRIVER_NUM_LTC2941, 6, 0);
}



int ltc2943_read_status_sync(void) {
    int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_read_status();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_configure_sync(interrupt_pin_conf_e int_pin, uint16_t prescaler, adc_mode_e adc) {
    int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_configure(int_pin, prescaler, adc);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_reset_charge_sync(void) {
    int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_reset_charge();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_set_high_threshold_sync(uint16_t threshold) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_set_high_threshold(threshold);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_set_low_threshold_sync(uint16_t threshold) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_set_low_threshold(threshold);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_get_charge_sync(void) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_get_charge();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.charge;
}

int ltc2943_get_voltage_sync(void) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_get_voltage();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.charge;
}

int ltc2943_get_current_sync(void) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_get_current();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.charge;
}

int ltc2943_shutdown_sync(void) {
	int err;
    result.fired = false;

    err = ltc2943_set_callback(ltc2943_cb, (void*) &result);
    if (err < 0) return err;

    err = ltc2943_shutdown();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return 0;
}

int ltc2943_convert_to_voltage_mv (int v) {
    return 23.6*(v/(float)0xFFFF)*1000;
}

int ltc2943_convert_to_current_ua (int c, int Rsense) {
    return (int)((60.0/Rsense)*((c-0x7FFF)/(float)0x7FFF)*1000000.0);
}
