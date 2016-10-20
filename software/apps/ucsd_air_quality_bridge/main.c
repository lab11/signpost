#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "gpio.h"
#include "i2c_master_slave.h"
#include "timer.h"
#include "tock.h"

#define STROBE 4
#define RESET 5
#define POWER 6

#define LED3 8
#define LED4 9
#define LED1 10
#define LED2 11

#define UCSD_AQ_I2C_ADDR 0x35

uint8_t txbuf[32] = {0};

static bool fired = false;
static char uart_rx_buf[128];
void uart_rx_callback (
		__attribute__ ((unused)) int len,
		__attribute__ ((unused)) int y,
		__attribute__ ((unused)) int z,
		__attribute__ ((unused)) void* userdata) {
	fired = true;

	memcpy(txbuf, uart_rx_buf, 3);
}

static void blink(unsigned duration_ms) {
	gpio_clear(LED1);
	gpio_clear(LED2);
	gpio_clear(LED3);
	gpio_clear(LED4);
	delay_ms(duration_ms);
	gpio_set(LED1);
	gpio_set(LED2);
	gpio_set(LED3);
	gpio_set(LED4);
}

int main () {
	// LEDs used for debug only
	gpio_enable_output(LED1);
	gpio_enable_output(LED2);
	gpio_enable_output(LED3);
	gpio_enable_output(LED4);

	// Setup I2C TX buffer
	txbuf[0] = UCSD_AQ_I2C_ADDR;  // My address
	txbuf[1] = 0x01;              // Message type

	// Set buffer for I2C messages
	i2c_master_slave_set_master_write_buffer(txbuf, 32);

	// Set out slave address (though we don't listen)
	i2c_master_slave_set_slave_address(UCSD_AQ_I2C_ADDR);

	// Debug: Blink on boot
	blink(1000);

	while (1) {
		getauto(uart_rx_buf, 128, uart_rx_callback, NULL);
		yield_for(&fired);
		fired = false; // feels like yield_for should do this

		// Debug: Quick blink every packet
		blink(10);

		// Forward on the payload
		i2c_master_slave_write_sync(0x20, 5);
	}
}
