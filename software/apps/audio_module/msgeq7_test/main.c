
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

#define MSGEQ7_RESET_PIN   6
#define MSGEQ7_STROBE_PIN  5
#define MSGEQ7_ADC_CHANNEL 0

//XXX: testing
uint8_t master_write_buf[20];
#define GREEN_LED 0
#define RED_LED 1

int main (void) {
  int err = SUCCESS;
  printf("[TEST] Audio ADC\n");

  // initialize the signpost bus
  err = signpost_initialization_module_init(
    i2c_address,
    SIGNPOST_INITIALIZATION_NO_APIS);
  if (err < SUCCESS) {
    printf("Signbus initialization failed\n");
    return FAIL;
  }

  // setup GPIO pins
  gpio_enable_output(MSGEQ7_RESET_PIN);
  gpio_enable_output(MSGEQ7_STROBE_PIN);
  gpio_clear(MSGEQ7_RESET_PIN);
  gpio_clear(MSGEQ7_STROBE_PIN);

  // initialize ADC
  err = adc_initialize();
  if (err < SUCCESS) {
    printf("Initialize errored: %d\n", err);
  }

  printf("Sampling data\n");
  while (true) {

    {
      // MAGIC - from original script
      delay_ms(1);
      gpio_set(MSGEQ7_STROBE_PIN);
      gpio_set(MSGEQ7_RESET_PIN);
      delay_ms(1);
      gpio_clear(MSGEQ7_STROBE_PIN);
      delay_ms(1);
      gpio_clear(MSGEQ7_RESET_PIN);
      gpio_set(MSGEQ7_STROBE_PIN);
      delay_ms(1);
      gpio_clear(MSGEQ7_STROBE_PIN);

      for(uint8_t i = 0; i < 6; i++) {
        uint16_t data = (uint16_t)adc_read_single_sample(0);
        master_write_buf[2+i*2] = (uint8_t)((data >> 8) & 0xff);
        master_write_buf[2+i*2+1] = (uint8_t)(data & 0xff);
        delay_ms(1);
        gpio_set(MSGEQ7_STROBE_PIN);
        delay_ms(1);
        gpio_clear(MSGEQ7_STROBE_PIN);
      }
      uint16_t data = (uint16_t)adc_read_single_sample(0);
      master_write_buf[14] = (uint8_t)((data >> 8) & 0xff);
      master_write_buf[15] = (uint8_t)(data & 0xff);

      //give some indication of volume to the user
      if((uint16_t)((master_write_buf[6] << 8) + master_write_buf[7]) > 400) {
        //turn on green LED
        led_on(GREEN_LED);
      } else {
        led_off(GREEN_LED);
      }
      if((uint16_t)((master_write_buf[6] << 8) + master_write_buf[7]) > 3500) {
        //turn on red LED
        led_on(RED_LED);
      } else {
        led_off(RED_LED);
      }
    }

    /*
    // read data from ADC
    err = adc_read_single_sample(3);
    if (err < SUCCESS) {
      printf("ADC read error: %d\n", err);
    }
    uint16_t sample = err & 0xFFFF;
    //printf("%d\n", data);
    */
  }
}

