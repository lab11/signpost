
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <adc.h>
#include <console.h>
#include <gpio.h>
#include <led.h>
#include <timer.h>
#include <tock.h>

#include "app_watchdog.h"
#include "signpost_api.h"
#include "simple_post.h"

#define ZERO_MAGNITUDE 2048 // middle value for a 12-bit ADC

#define SAMPLE_COUNT 64 // powers of two please!
static uint16_t sample_buf[SAMPLE_COUNT] = {ZERO_MAGNITUDE};

bool wrote = false;

int main (void) {
  int err = SUCCESS;
  printf("[Audio Module] Quiet Detector\n");

  // initialize ADC
  err = adc_initialize();
  if (err < SUCCESS) {
    printf("Initialize errored: %d\n", err);
  }

  printf("Sampling data\n");
  uint8_t sample_index = 0;
  uint16_t min_amplitude = 100;
  while (true) {

    // read data from ADC
    err = adc_read_single_sample(3);
    if (err < SUCCESS) {
      printf("ADC read error: %d\n", err);
    }
    uint16_t sample = err & 0xFFFF;

    // calculate amplitude of sample
    uint16_t amplitude = abs(sample-ZERO_MAGNITUDE);
    sample_buf[sample_index] = amplitude;

    // calculate average amplitude over last N samples
    uint32_t amplitude_sum = 0;
    for (int i=0; i<SAMPLE_COUNT; i++) {
      amplitude_sum += sample_buf[i];
    }
    uint16_t average_amplitude = amplitude_sum / SAMPLE_COUNT;

    if (average_amplitude < 100) {
      // also POST that it was quiet
      if (!wrote) {
        printf("Silence detected\n");
        wrote = true;
      }

      // keep track of how quiet it ever gets
      if (average_amplitude < min_amplitude) {
        min_amplitude = average_amplitude;
      }
    } else {
      if (wrote) {
        printf("Min amplitude: %d\n", min_amplitude);
      }

      // reset wrote
      wrote = false;
      min_amplitude = 100;
    }

    // increment sample index
    sample_index++;
    if (sample_index >= SAMPLE_COUNT) {
      sample_index = 0;
    }
  }
}

