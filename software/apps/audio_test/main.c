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
#define	POWER 6

int main () {
	gpio_enable_output(STROBE);
	gpio_enable_output(RESET);
	gpio_enable_output(POWER); 

	gpio_clear(POWER);
	gpio_clear(STROBE);
	gpio_clear(RESET);

	
	delay_ms(1000);
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
			delay_ms(1);
			gpio_set(STROBE);
			delay_ms(1);
			gpio_clear(STROBE);
		}
  	}
}
