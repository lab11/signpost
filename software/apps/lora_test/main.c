#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "iM880A_RadioInterface.h"
#include "timer.h"
#include "tock.h"

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
