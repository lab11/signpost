#pragma once

#include "tock.h"

#define DRIVER_NUM_I2C_SELECTOR 101

// Set a callback for this driver.
int i2c_selector_set_callback(subscribe_cb callback, void* callback_args);

// Channels is a bit mask, where a "1" means activate that I2C channel.
// The uint32_t is bit-packed such that the first selector is the lowest
// four bits, the next is the next four bits, etc.
int i2c_selector_select_channels(uint32_t channels);

// Disable all channels on all I2C selectors.
int i2c_selector_select_channels();

// Read interrupts on all I2C selectors
int i2c_selector_read_interrupts();

int i2c_selector_read_selected();
