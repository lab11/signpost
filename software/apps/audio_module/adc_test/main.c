
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include <console.h>
#include "gpio.h"
#include "led.h"
#include "adc.h"
#include "app_watchdog.h"
#include "timer.h"

#include "signpost_api.h"

static const uint8_t i2c_address = 0x33;
#define ZERO_MAGNITUDE 2048 // middle value for a 12-bit ADC

#define SAMPLE_COUNT 64 // powers of two please!
static uint16_t sample_buf[SAMPLE_COUNT] = {ZERO_MAGNITUDE};

int main (void) {
  int err = SUCCESS;
  printf("[TEST] Audio ADC\n");

  // initialize the signpost bus
  signpost_initialization_module_init(
    i2c_address,
    SIGNPOST_INITIALIZATION_NO_APIS);

  // initialize ADC
  err = adc_initialize();
  if (err < SUCCESS) {
    printf("Initialize errored: %d\n", err);
  }

  printf("Sampling data\n");
  uint8_t sample_index = 0;
  while (true) {

    // read data from ADC
    err = adc_read_single_sample(3);
    if (err < SUCCESS) {
      printf("ADC read error: %d\n", err);
    }
    uint16_t sample = err & 0xFFFF;
    //printf("%d\n", data);

    // calculate amplitude of sample
    uint16_t amplitude = abs(sample-ZERO_MAGNITUDE);
    sample_buf[sample_index] = amplitude;

    // calculate average amplitude over last N samples
    uint32_t amplitude_sum = 0;
    for (int i=0; i<SAMPLE_COUNT; i++) {
      amplitude_sum += sample_buf[i];
    }
    uint16_t average_amplitude = amplitude_sum / SAMPLE_COUNT;
    //printf("%d\n", average_amplitude);

    if (average_amplitude > 1000) {
      printf("LOUD %d\n", sample_index);
    }

    // increment sample index
    sample_index++;
    if (sample_index >= SAMPLE_COUNT) {
      sample_index = 0;
    }
  }
}

