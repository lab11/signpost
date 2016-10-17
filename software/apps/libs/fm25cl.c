
#include "tock.h"
#include "fm25cl.h"


struct fm25cl_data {
  bool fired;
};

static struct fm25cl_data result = { .fired = false };

// Internal callback for faking synchronous reads
static void fm25cl_cb(__attribute__ ((unused)) int callback_type,
                      __attribute__ ((unused)) int len,
                      __attribute__ ((unused)) int unused,
                      void* ud) {
  struct fm25cl_data* result = (struct fm25cl_data*) ud;
  result->fired = true;
}

int fm25cl_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_FM25CL, 0, callback, callback_args);
}

int fm25cl_set_read_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_FM25CL, 0, (void*) buffer, len);
}

int fm25cl_set_write_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_FM25CL, 1, (void*) buffer, len);
}

int fm25cl_read_status() {
	return command(DRIVER_NUM_FM25CL, 0, 0);
}

int fm25cl_read(uint16_t address, uint16_t len) {
	uint32_t a = (((uint32_t) len) << 16) | address;
	return command(DRIVER_NUM_FM25CL, 1, a);
}

int fm25cl_write(uint16_t address, uint16_t len) {
	uint32_t a = (((uint32_t) len) << 16) | address;
	return command(DRIVER_NUM_FM25CL, 2, a);
}

int fm25cl_read_status_sync() {
	int err;
    result.fired = false;

	err = fm25cl_set_callback(fm25cl_cb, (void*) &result);
	if (err < 0) return err;

	err = fm25cl_read_status();
	if (err < 0) return err;

	// Wait for the callback.
	yield_for(&result.fired);

	return 0;
}

int fm25cl_read_sync(uint16_t address, uint16_t len) {
	int err;
    result.fired = false;

	err = fm25cl_set_callback(fm25cl_cb, (void*) &result);
	if (err < 0) return err;

	err = fm25cl_read(address, len);
	if (err < 0) return err;

	// Wait for the callback.
	yield_for(&result.fired);

	return 0;
}

int fm25cl_write_sync(uint16_t address, uint16_t len) {
	int err;
    result.fired = false;

	err = fm25cl_set_callback(fm25cl_cb, (void*) &result);
	if (err < 0) return err;

	err = fm25cl_write(address, len);
	if (err < 0) return err;

	// Wait for the callback.
	yield_for(&result.fired);

	return 0;
}
