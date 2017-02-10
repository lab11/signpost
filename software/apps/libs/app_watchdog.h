#pragma once

#include "tock.h"

#define DRIVER_NUM_APPWATCHDOG 108

int app_watchdog_start(void);
int app_watchdog_stop(void);
int app_watchdog_tickle_app(void);
int app_watchdog_tickle_kernel(void);
int app_watchdog_set_app_timeout(int timeout);
int app_watchdog_set_kernel_timeout(int timeout);
int app_watchdog_reset_app(void);
