#pragma once

#include "tock.h"

#define DRIVER_NUM_LEDS 10

int led_on(int led_num);
int led_off(int led_num);
int led_toggle(int led_num);
