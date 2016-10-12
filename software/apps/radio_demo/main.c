#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

//nordic includes
#include "nrf.h"
#include <nordic_common.h>
#include <nrf_error.h>
#include <simple_ble.h>
#include <eddystone.h>
#include <simple_adv.h>
#include "multi_adv.h"

//tock includes
#include <tock.h>
#include "firestorm.h"
#include <tock_str.h>
#include "iM880A_RadioInterface.h"
#include "i2c_master_slave.h"

//definitions for the ble
#define DEVICE_NAME "Signpost"
#define PHYSWEB_URL "testURL"

#define ADV_SWITCH_MS 100

#define UMICH_COMPANY_IDENTIFIER 0x02E0

static simple_ble_config_t ble_config = {
	.platform_id 		= 0x00,
	.device_id			= DEVICE_ID_DEFAULT,
	.adv_name			= DEVICE_NAME,
	.adv_interval		= MSEC_TO_UNITS(50, UNIT_0_625_MS),
	.min_conn_interval	= MSEC_TO_UNITS(500, UNIT_1_25_MS),
	.max_conn_interval	= MSEC_TO_UNITS(1000, UNIT_1_25_MS),
};

//definitions for the i2c
#define BUFFER_SIZE 20
#define NUMBER_OF_MODULES 6

//i2c buffers
uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];

//array of the data we're going to send on the radios
uint8_t data_to_send[NUMBER_OF_MODULES][BUFFER_SIZE];

static void adv_config_eddystone () {
	eddystone_adv(PHYSWEB_URL, NULL);
}

static void adv_config_data() {
	static uint8_t i = 0;

	ble_advdata_manuf_data_t mandata;

	mandata.company_identifier = UMICH_COMPANY_IDENTIFIER;
	mandata.data.p_data = data_to_send[i];
	mandata.data.size = BUFFER_SIZE;
	
	simple_adv_manuf_data(&mandata);
	
	//on the next advertisement update
	i++;
}

static void i2c_master_slave_callback (
	int callback_type,
	int length __attribute__ ((unused)),
	int unused __attribute__ ((unused)),
	void * callback_args __attribute__ ((unused))) {

	//for now only take writes
	if(callback_type == CB_SLAVE_READ_REQUEST) {
		return;
	} else if (callback_type == CB_SLAVE_READ_COMPLETE) {
		return;
	} else if (callback_type == CB_SLAVE_WRITE) {
		//this is a valid sender ID
		if(slave_write_buf[0] < NUMBER_OF_MODULES) {
			//copy the data they wrote into their send array
			memcpy(data_to_send[slave_write_buf[0]], slave_write_buf, BUFFER_SIZE);
		}
	}
}

int main () {
	//configure the radios
	//lora
	uint16_t result = iM880A_Configure();

	//ble
	simple_ble_init(&ble_config);
	multi_adv_init(ADV_SWITCH_MS);
	multi_adv_register_config(adv_config_eddystone);
	multi_adv_register_config(adv_config_data);

	multi_adv_start();

	//configure the data array to send zeros with IDs
	for(uint8_t i = 0; i < NUMBER_OF_MODULES; i++) {
		data_to_send[i][0] = i;
		for(uint8_t j = 1; j < BUFFER_SIZE; j++) {
			data_to_send[i][j] = 0;
		}
	}

	//low configure i2c slave to listen
	i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
	i2c_master_slave_set_slave_address(0xFE);
//
	i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
	i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

	//listen
	i2c_master_slave_listen();

	while(1) {
		static uint8_t i = 0;

		uint8_t ID = i % NUMBER_OF_MODULES;

		result = iM880A_SendRadioTelegram(data_to_send[ID],BUFFER_SIZE);
		delay_ms(2000);

		i++;
	}
}
