#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <timer.h>
#include <tock.h>

#include "iM880A_RadioInterface.h"

const char* message = "Hello from LAB12";

int main (void) {
	iM880A_Configure();
	delay_ms(1000);
	while (1) {
		iM880A_PingRequest();
		delay_ms(1000);
		iM880A_SendRadioTelegram((uint8_t*)message,strlen(message));
		delay_ms(1000);
	}
}
