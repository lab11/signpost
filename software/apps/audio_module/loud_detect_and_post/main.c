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
#include "simple_post.h"
#include "signpost_api.h"

static const uint8_t i2c_address = 0x33;
#define ZERO_MAGNITUDE 2048 // middle value for a 12-bit ADC

#define SAMPLE_COUNT 64 // powers of two please!
static uint16_t sample_buf[SAMPLE_COUNT] = {ZERO_MAGNITUDE};

// only post once per LOUD event
static bool posted = false;

static void post_over_http (void) {
  // post sensor data over HTTP and get response

  // URL for an HTTP POST testing service
  const char* url = "httpbin.org/post";

  // http post data
  printf("--POSTing data--\n");
  int response = simple_octetstream_post(url, (uint8_t*)"LOUD", 5);
  if (response < SUCCESS) {
    printf("Error posting: %d\n", response);
  } else {
    printf("\tResponse: %d\n", response);
  }

  // don't try again until next loud noise
  posted = true;
}

int main (void) {
  int err = SUCCESS;
  printf("[Audio Module] Loudness Detector\n");

  // initialize the signpost bus
  int rc;
  do {
    rc = signpost_initialization_module_init(
      i2c_address,
      SIGNPOST_INITIALIZATION_NO_APIS);
    if (rc < 0) {
      printf(" - Error initializing bus (code: %d). Sleeping 5s\n", rc);
      delay_ms(5000);
    }
  } while (rc < 0);

  // initialize ADC
  err = adc_initialize();
  if (err < SUCCESS) {
    printf("ADC initialization errored: %d\n", err);
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

    // calculate amplitude of sample
    uint16_t amplitude = abs(sample-ZERO_MAGNITUDE);
    sample_buf[sample_index] = amplitude;

    // calculate average amplitude over last N samples
    uint32_t amplitude_sum = 0;
    for (int i=0; i<SAMPLE_COUNT; i++) {
      amplitude_sum += sample_buf[i];
    }
    uint16_t average_amplitude = amplitude_sum / SAMPLE_COUNT;

    if (average_amplitude > 1000) {
      // also POST that it was loud
      if (!posted) {
        printf("Loudness detected. Amplitude %d\n", average_amplitude);
        post_over_http();
      }
    } else {
      // reset posted
      posted = false;
    }

    // increment sample index
    sample_index++;
    if (sample_index >= SAMPLE_COUNT) {
      sample_index = 0;
    }
  }
}

