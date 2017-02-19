#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_watchdog.h"
#include "console.h"
#include "gps.h"
#include "gpio.h"
#include "gpio_async.h"
#include "i2c_master_slave.h"
#include "signpost_api.h"
#include "timer.h"
#include "tock.h"


static uint8_t src;
static uint8_t rx_buffer[2048];

static bool message_sent = false;

static void tx_callback (
            __attribute__ ((unused)) int u1,
            __attribute__ ((unused)) int u2,
            __attribute__ ((unused)) int u3,
            __attribute__ ((unused)) void* userdata) {

    message_sent = true;
}

static bool waiting_for_response = 0;

static void rx_callback (
            int len,
            __attribute__ ((unused)) int u2,
            __attribute__ ((unused)) int u3,
            __attribute__ ((unused)) void* userdata) {

    waiting_for_response = 0;
    signpost_networking_post_reply(src, rx_buffer, len);
}

static void watchdog_timer_cb (
            __attribute__ ((unused)) int len,
            __attribute__ ((unused)) int u2,
            __attribute__ ((unused)) int u3,
            __attribute__ ((unused)) void* userdata) {

    app_watchdog_tickle_kernel();
}


static void networking_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, size_t message_length, uint8_t* message) {

    if (api_type != NetworkingApiType) {
        signpost_api_error_reply(source_address, api_type, message_type);
        return;
    }

    src = source_address;

  if (frame_type == NotificationFrame) {
    // XXX unexpected, drop
  } else if (frame_type == CommandFrame) {

      if (message_type == NetworkingPostMessage) {

        static char d[2];
        d[0] = '$';
        message_sent = false;
        allow(DRIVER_NUM_GPS, 1, (void*)d, 1);
        subscribe(DRIVER_NUM_GPS, 1, tx_callback, NULL);
        yield_for(&message_sent);

        d[0] = message_length & 0xff;
        d[1] = ((message_length & 0xff00) >> 8);
        message_sent = false;
        allow(DRIVER_NUM_GPS, 1, (void*)d, 2);
        subscribe(DRIVER_NUM_GPS, 1, tx_callback, NULL);
        yield_for(&message_sent);

        message_sent = false;
        allow(DRIVER_NUM_GPS, 1, (void*)message, message_length);
        subscribe(DRIVER_NUM_GPS, 1, tx_callback, NULL);
        yield_for(&message_sent);

        waiting_for_response = 1;
        getauto((char *)rx_buffer,4096, rx_callback,NULL);
    }

  } else if (frame_type == ResponseFrame) {
    // XXX unexpected, drop
  } else if (frame_type == ErrorFrame) {
    // XXX unexpected, drop
  }
}


int main (void) {
  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost

  // Install hooks for the signpost APIs we implement
  static api_handler_t networking_handler = {NetworkingApiType, networking_api_callback};
  static api_handler_t* handlers[] = {&networking_handler, NULL};
  signpost_initialization_module_init(ModuleAddressRadio, handlers);

  message_sent = false;
  static char d1[3];
  d1[0] = '#';
  d1[1] = 'r';
  allow(DRIVER_NUM_GPS, 1, (void*)d1, 2);
  subscribe(DRIVER_NUM_GPS, 1, tx_callback, NULL);
  yield_for(&message_sent);


    //should probably actually setup a watchdog at some point
  ////////////////////////////////////////////////
  // Setup watchdog
  timer_subscribe(watchdog_timer_cb,NULL);
  timer_start_repeating(160);
  app_watchdog_set_kernel_timeout(500);
  app_watchdog_start();

  /*while(1) {
        delay_ms(2000);
        message_sent = false;
        static char d2[3];
        d2[0] = '#';
        d2[1] = 'w';
        allow(DRIVER_NUM_GPS, 1, (void*)d2, 2);
        subscribe(DRIVER_NUM_GPS, 1, tx_callback, NULL);
        yield_for(&message_sent);
  }*/
}
