#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "gpio.h"
#include "timer.h"
#include "tock.h"

#define STROBE 4
#define RESET 5
#define POWER 6

#define LED3 8
#define LED4 9
#define LED1 10
#define LED2 11

static bool fired = false;
static char uart_rx_buf[128];
void uart_rx_callback (
		__attribute__ ((unused)) int len,
		__attribute__ ((unused)) int y,
		__attribute__ ((unused)) int z,
		__attribute__ ((unused)) void* userdata) {
	fired = true;
}

int main () {
	gpio_enable_output(LED1);
	gpio_enable_output(LED2);
	gpio_enable_output(LED3);
	gpio_enable_output(LED4);

	//while (1) {
		gpio_clear(LED1);
		gpio_clear(LED2);
		gpio_clear(LED3);
		gpio_clear(LED4);
		delay_ms(1000);
		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
		gpio_set(LED4);
		delay_ms(1000);
	//}


	while (1) {
		getauto(uart_rx_buf, 128, uart_rx_callback, NULL);
		yield_for(&fired);
		fired = false; // feels like yield_for should do this

		gpio_clear(LED1);
		gpio_clear(LED2);
		gpio_clear(LED3);
		gpio_clear(LED4);
		delay_ms(10);
		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
		gpio_set(LED4);
	}
}
