#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_watchdog.h"
#include "signpost_api.h"
#include "tock.h"
#include "console.h"



int main (void) {
  putstr("[Http_test] ** Main App **\n");

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost
  signpost_initialization_module_init(0x50, NULL);
  putstr("Initialized\n");

  uint8_t test[600];

  while(1) {
      delay_ms(1000);
      const char* url = "posttestserver.com/post.php";
      http_request r;
      //the library will automatically throw in content-length
      //if you don't, because it's annoying to do yourself
      r.num_headers = 1;
      http_header h;
      h.header = "content-type";
      h.value = "application/json";
      r.headers = &h;
      r.body_len = 600;
      r.body = test;

      putstr("About to send\n");
      http_response result = signpost_networking_post(url, r);
      putstr("Returned\n");
      delay_ms(5000);
  }


    //should probably actually setup a watchdog at some point
  ////////////////////////////////////////////////
  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("Everything intialized\n");
}
