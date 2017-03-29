#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gpio.h>
#include <led.h>
#include <timer.h>
#include <tock.h>

#include "signpost_api.h"

static const uint8_t random_i2c_address = 0x51;

static signpost_timelocation_time_t time;
static uint8_t new_time = 1;
static uint8_t flash_led = false;

//assuming that no time_location request takes longer than ~1s
//time should be
//if(new_time == 1) {
//time + timer_read
//}
//
static uint16_t last_pps_time = 0;
static uint16_t pin_time = 0;
static uint8_t got_int = 0;

static void timer_callback (__attribute ((unused)) int pin_num,
        __attribute ((unused)) int arg2,
        __attribute ((unused)) int arg3,
        __attribute ((unused)) void* userdata) {
}


static void pps_callback (int pin_num,
        __attribute ((unused)) int arg2,
        __attribute ((unused)) int arg3,
        __attribute ((unused)) void* userdata) {

    if(pin_num == 2) {
        last_pps_time = timer_read();

        //now let's query time from the controller again
        new_time = 0;

        if(flash_led) {
            led_toggle(0);
            flash_led = false;
        }
    } else if (pin_num == 4) {
        pin_time = timer_read();
        got_int = 1;
        float seconds;
        if(new_time) {
            seconds = time.seconds + (last_pps_time - pin_time)/16000.0;
        } else {
            seconds = 1 + time.seconds + (last_pps_time - pin_time)/16000.0;
        }
        printf("Interrupt occurred at %d:%f",time.minutes,seconds);
    }
}

int main (void) {
  printf("\n\n[Test] Synchronization\n");


    //initialize signpost API
  int rc = signpost_initialization_module_init(
      random_i2c_address,
      SIGNPOST_INITIALIZATION_NO_APIS);
  if (rc < SUCCESS) {
    printf("Signpost initialization errored: %d\n", rc);
  }

  led_off(0);

  //setup a callback for pps
  gpio_enable_interrupt(2, PullNone, RisingEdge);
  gpio_enable_interrupt(4, PullDown, RisingEdge);
  gpio_interrupt_callback(pps_callback, NULL);

  //this is just to make sure the timer is running
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(2000);

  while (true) {
    if(new_time == 0) {
        rc = signpost_timelocation_get_time(&time);
        if(rc >= 0) {
            new_time = 1;
            if((time.seconds % 10) == 0) {
                flash_led = true;
            }
        }
    }
    yield();
  }
}
