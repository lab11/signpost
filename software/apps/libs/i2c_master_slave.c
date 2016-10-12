#include <firestorm.h>
#include <tock.h>
#include <i2c_master_slave.h>




int i2c_master_slave_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_I2CMASTERSLAVE, 0, callback, callback_args);
}

int i2c_master_slave_set_master_write_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_I2CMASTERSLAVE, 0, (void*) buffer, len);
}

int i2c_master_slave_set_master_read_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_I2CMASTERSLAVE, 1, (void*) buffer, len);
}

int i2c_master_slave_set_slave_read_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_I2CMASTERSLAVE, 2, (void*) buffer, len);
}

int i2c_master_slave_set_slave_write_buffer(uint8_t* buffer, uint32_t len) {
	return allow(DRIVER_NUM_I2CMASTERSLAVE, 3, (void*) buffer, len);
}

int i2c_master_slave_write(uint8_t address, uint8_t length) {
	uint32_t a = (((uint32_t) length) << 16) | address;
	return command(DRIVER_NUM_I2CMASTERSLAVE, 0, a);
}

int i2c_master_slave_read(uint16_t address, uint16_t len) {
	uint32_t a = (((uint32_t) len) << 16) | address;
	return command(DRIVER_NUM_I2CMASTERSLAVE, 1, a);
}

int i2c_master_slave_listen() {
	return command(DRIVER_NUM_I2CMASTERSLAVE, 2, 0);
}

int i2c_master_slave_set_slave_address(uint8_t address) {
	return command(DRIVER_NUM_I2CMASTERSLAVE, 5, address);
}
