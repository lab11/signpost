
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <console.h>
#include "firestorm.h"
#include "gpio.h"
#include "adc.h"
#include "msgeq7.h"
#include "i2c_master_slave.h"

#define STROBE 4
#define RESET 5
#define	POWER 6

#define BUFFER_SIZE 20


uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];


static void i2c_master_slave_callback(int callback_type, int length, int unused, void* callback_args) {
    return;
}


/*static void  timer_callback( int callback_type , int channel, int data, void* callback_args) {
    i2c_master_slave_write(0x22,16);
}*/

int main () {

    //init i2c
    i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_slave_address(0x33);

    i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
    i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
    i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);
    i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);

    i2c_master_slave_listen();

    master_write_buf[0] = 0x33;
    master_write_buf[1] = 0x01;

    //init adc
    //adc_set_callback(adc_callback, (void*)&result);
    adc_initialize();
	gpio_enable_output(STROBE);
	gpio_enable_output(RESET);
	gpio_enable_output(POWER);

	gpio_clear(POWER);
	gpio_clear(STROBE);
	gpio_clear(RESET);

    //timer_subscribe(timer_callback, NULL);
    //timer_start_repeating(1000);
    //i2c_master_slave_write(0x22,16);

	delay_ms(1000);
    uint8_t count = 50;
  	while (1) {
		delay_ms(1);
		gpio_set(STROBE);
		gpio_set(RESET);
		delay_ms(1);
		gpio_clear(STROBE);
		delay_ms(1);
		gpio_clear(RESET);
		gpio_set(STROBE);
		delay_ms(1);
		gpio_clear(STROBE);

		for(uint8_t i = 0; i < 6; i++) {
            uint16_t data = (uint16_t)adc_read_single_sample(0);
            master_write_buf[2+i*2] = (uint8_t)((data >> 8) & 0xff);
            master_write_buf[2+i*2+1] = (uint8_t)(data & 0xff);
			delay_ms(1);
			gpio_set(STROBE);
			delay_ms(1);
			gpio_clear(STROBE);
		}
        uint16_t data = (uint16_t)adc_read_single_sample(0);
        master_write_buf[14] = (uint8_t)((data >> 8) & 0xff);
        master_write_buf[15] = (uint8_t)(data & 0xff);
        count++;
        if(count >= 50) {
            i2c_master_slave_write(0x22,16);
            count = 0;
        }
  	}
}
