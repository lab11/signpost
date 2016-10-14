#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

//tock includes
#include <tock.h>
#include "firestorm.h"
#include <tock_str.h>
//#include "msgeq7.h"
#include "i2c_master_slave.h"

#define BUFFER_SIZE 20
//i2c buffers
uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];

uint16_t result[7];

static void i2c_master_slave_callback (
	int callback_type __attribute__ ((unused)),
	int length __attribute__ ((unused)),
	int unused __attribute__ ((unused)),
	void * callback_args __attribute__ ((unused))) {
	return;
}

int main () {
	//low configure i2c slave to listen
	i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
	i2c_master_slave_set_slave_address(0x33);
//
	i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
	i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

	//listen
	i2c_master_slave_listen();

	//msgeq7_initialize(4, 5, 0);
	master_write_buf[0] = 0x33;
	master_write_buf[1] = 0x00;

	while(1) {
	//	msgeq7_get_values(result);
	//	memcpy(master_write_buf+2, (uint8_t* )result, 14);
		i2c_master_slave_write(0x22, 16);
		delay_ms(1000);
		yield();
	}
}
