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

int main (void) {
  int err = SUCCESS;
  printf("[Audio Module] Simple ADC test\n");

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
    printf("%d\n", sample);

    delay_ms(1);
  }
}

