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
//#include "multi_adv.h"

//tock includes
#include <tock.h>
#include "firestorm.h"
#include <tock_str.h>
#include "iM880A_RadioInterface.h"
#include "i2c_master_slave.h"
#include "radio_module.h"
#include "gpio.h"

//definitions for the ble
#define DEVICE_NAME "Signpost"
#define PHYSWEB_URL "testURL"


#define UMICH_COMPANY_IDENTIFIER 0x02E0

static simple_ble_config_t ble_config = {
	.platform_id 		= 0x00,
	.device_id			= DEVICE_ID_DEFAULT,
	.adv_name			= DEVICE_NAME,
	.adv_interval		= MSEC_TO_UNITS(150, UNIT_0_625_MS),
	.min_conn_interval	= MSEC_TO_UNITS(500, UNIT_1_25_MS),
	.max_conn_interval	= MSEC_TO_UNITS(1250, UNIT_1_25_MS),
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

static void adv_config_name() {
	simple_adv_only_name();
}

static void adv_config_data() {
	static uint8_t i = 0;

	static ble_advdata_manuf_data_t mandata;

	mandata.company_identifier = UMICH_COMPANY_IDENTIFIER;
	mandata.data.p_data = data_to_send[0];
	mandata.data.size = 1;

	simple_adv_manuf_data(&mandata);

	i++;
	if(i >= NUMBER_OF_MODULES) {
		i = 0;
	}
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
		switch(slave_write_buf[0]) {
		case 0x20:
			if(slave_write_buf[1] == 0x01) {
				memcpy(data_to_send[0], slave_write_buf, BUFFER_SIZE);
			} else if (slave_write_buf[1] == 0x02) {
				memcpy(data_to_send[1], slave_write_buf, BUFFER_SIZE);
			} else {
				//this shouldn't happen
			}
		break;
		case 0x31:
			memcpy(data_to_send[2], slave_write_buf, BUFFER_SIZE);
		break;
		case 0x32:
			memcpy(data_to_send[3], slave_write_buf, BUFFER_SIZE);
		break;
		case 0x33:
			memcpy(data_to_send[4], slave_write_buf, BUFFER_SIZE);
		break;
		case 0x34:
			memcpy(data_to_send[5], slave_write_buf, BUFFER_SIZE);
		break;
		default:
			//this shouldn't happen
		break;
		}
	}
}

void ble_address_set() {
	//this has to be here I promise
}

void ble_error(uint32_t error_code) {
	//this has to be here too
    putstr("BLE Error");
    char buf[20];
    snprintf(buf, 20, "err/line: %d", error_code);
    putnstr(buf, 20);
}

void ble_evt_connected(ble_evt_t* p_ble_evt) {
	//this might also need to be here
}

void ble_evt_disconnected(ble_evt_t* p_ble_evt) {
	//this too
}

void ble_evt_user_handler (ble_evt_t* p_ble_evt) {
	//and maybe this
}

static void timer_callback (
	int callback_type __attribute__ ((unused)),
	int length __attribute__ ((unused)),
	int unused __attribute__ ((unused)),
	void * callback_args __attribute__ ((unused))) {

	static uint8_t i = 0;

//	iM880A_SendRadioTelegram(data_to_send[i],BUFFER_SIZE);
	if(i == 5) {
		putstr("eddystone adv\n");
		eddystone_adv(PHYSWEB_URL, NULL);
	} else {
		putstr("data adv\n");
		adv_config_data();
	}

	i++;
	if(i >= NUMBER_OF_MODULES) {
		i = 0;
	}
}

int main () {
	//configure the radios
	//lora
//	uint16_t result = iM880A_Configure();
//
    gpio_enable_output(BLE_POWER);
    gpio_set(BLE_POWER);
    delay_ms(10);
    gpio_clear(BLE_POWER);

	putstr("started app\n");
	//ble
	simple_ble_init(&ble_config);
	putstr("init ble\n");

	//setup a tock timer to
	eddystone_adv(PHYSWEB_URL,NULL);
	putstr("started physweb adv\n");

	//configure the data array to send zeros with IDs
	data_to_send[0][0] = 0x20;
	data_to_send[1][0] = 0x20;
	data_to_send[1][1] = 0x01;

	data_to_send[2][0] = 0x31;
	data_to_send[3][0] = 0x32;
	data_to_send[4][0] = 0x33;
	data_to_send[5][0] = 0x34;

	//low configure i2c slave to listen
	i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
	i2c_master_slave_set_slave_address(0x22);

	i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
	i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);
	i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);

	//listen
	i2c_master_slave_listen();

	//setup timer
	timer_subscribe(timer_callback, NULL);
	timer_start_repeating(150);

	while(1) {
		yield();
	}
}
