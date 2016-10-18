#include <stdint.h>

#include "tock.h"
#include "app_watchdog.h"

#define DRIVER_NUM_APPWATCHDOG 108

int app_watchdog_start() {
	return command(DRIVER_NUM_APPWATCHDOG, 4, 0);
}

int app_watchdog_stop() {
	return command(DRIVER_NUM_APPWATCHDOG, 5, 0);
}

int app_watchdog_tickle_app() {
	return command(DRIVER_NUM_APPWATCHDOG, 0, 0);
}

int app_watchdog_tickle_kernel() {
	return command(DRIVER_NUM_APPWATCHDOG, 1, 0);
}

int app_watchdog_set_app_timeout(int timeout) {
	return command(DRIVER_NUM_APPWATCHDOG, 1, timeout);
}

int app_watchdog_set_kernel_timeout(int timeout) {
	return command(DRIVER_NUM_APPWATCHDOG, 2, timeout);
}
