#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include "gpio.h"

#define STROBE 4
#define RESET 5
#define POWER 6

#define LED3 8
#define LED4 9
#define LED1 10
#define LED2 11

int main () {
	gpio_enable_output(LED1);
	gpio_enable_output(LED2);
	gpio_enable_output(LED3);
	gpio_enable_output(LED4);

	while (1) {
		gpio_set(LED1);
		gpio_set(LED2);
		gpio_set(LED3);
		gpio_set(LED4);
		delay_ms(1000);
		gpio_clear(LED1);
		gpio_clear(LED2);
		gpio_clear(LED3);
		gpio_clear(LED4);
		delay_ms(1000);
	}
}
