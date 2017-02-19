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
#include "timer.h"



int main (void) {
  putstr("[Http_test] ** Main App **\n");

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost
  signpost_initialization_module_init(0x28, NULL);
  putstr("Initialized\n");

  uint8_t test[200];

      const char* url = "posttestserver.com/post.php";
      http_request r;
      //the library will automatically throw in content-length
      //if you don't, because it's annoying to do yourself
      r.num_headers = 1;
      http_header h;
      h.header = "content-type";
      h.value = "application/json";
      r.headers = &h;
      r.body_len = 20;
      r.body = test;

      //prep a place to put responses
      http_response r2;
      char reason[50];
      r2.num_headers = 1;
      char h1[20];
      char v1[20];
      http_response_header hr;
      hr.header = h1;
      hr.header_len = 20;
      hr.value = v1;
      hr.value_len = 20;
      r2.headers = &hr;
      r2.reason = reason;
      r2.reason_len = 50;
      r2.body_len = 200;
      r2.body = test;

    while(1) {
        delay_ms(1000);
        putstr("About to send\n");
        int result = signpost_networking_post(url, r, &r2);
        if(result == 0) {
            printf("Status code is: %d\n", r2.status);
            printf("Reason len is: %d\n", r2.reason_len);
            printf("Reason is: %.*s\n", r2.reason_len, r2.reason);
            printf("Num Headers is: %d\n", r2.num_headers);
            printf("Header_len 1 is: %d\n", r2.headers[0].header_len);
            printf("Header 1 is: %.*s\n", r2.headers[0].header_len,r2.headers[0].header);
            printf("Value_len 1 is: %d\n", r2.headers[0].value_len);
            printf("Value 1 is: %.*s\n", r2.headers[0].value_len,r2.headers[0].value);
            printf("Body_len is: %d\n", r2.body_len);
            printf("Body is: %.*s\n", r2.body_len, r2.body);
            //printf("Reason is: %.*s\n", 10, r2.reason);
        } else {
            printf("There was an error\n");
        }
    }

    //should probably actually setup a watchdog at some point
  ////////////////////////////////////////////////
  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("Everything intialized\n");
}
