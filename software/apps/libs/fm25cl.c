#include <firestorm.h>
#include <tock.h>
#include <fm25cl.h>




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
