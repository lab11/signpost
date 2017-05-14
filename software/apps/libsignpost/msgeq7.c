#include <stdint.h>
#include <stdbool.h>

#include <adc.h>
#include <gpio.h>
#include <timer.h>

#include "msgeq7.h"

static uint8_t strobe;
static uint8_t reset;
static uint8_t out;

static uint16_t adc_data;
static bool adc_done;

static void adc_callback(int callback_type __attribute__ ((unused)),
							int channel __attribute__ ((unused)),
							int data,
							void* callback_args __attribute__ ((unused))) {

	adc_done = 1;
	adc_data = data;
}

void msgeq7_initialize(uint8_t strobe_pin,
						uint8_t reset_pin,
						uint8_t out_pin) {

	strobe = strobe_pin;
	reset = reset_pin;
	out = out_pin;

	gpio_enable_output(strobe);
	gpio_enable_output(reset);

	gpio_clear(strobe);
	gpio_clear(reset);

	adc_set_callback(adc_callback, NULL);
	adc_initialize();
}

void msgeq7_get_values(uint16_t* samples) {

	gpio_set(strobe);
	gpio_set(reset);
	delay_ms(1);
	gpio_clear(strobe);
	delay_ms(1);
	gpio_set(strobe);
	gpio_clear(reset);
	delay_ms(1);
	uint32_t i;
	for(i = 0; i < 7; i++) {
		//sample adc
		gpio_clear(strobe);
		delay_ms(1);

		adc_done = 0;
		adc_single_sample(out);
		while(adc_done == 0) {
			yield();
		}
		samples[i] = adc_data;

		delay_ms(1);
		gpio_set(strobe);
		delay_ms(1);
	}

	gpio_clear(strobe);
}
