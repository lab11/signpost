#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <timer.h>
#include <tock.h>

#include "app_watchdog.h"
#include "signpost_api.h"
#include "simple_post.h"


int main (void) {
  printf("[simple_networking_test] ** Main App **\n");

  int rc;

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost
  do {
    rc = signpost_initialization_module_init(0x28, NULL);
    if (rc < 0) {
      printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
      delay_ms(5000);
    }
  } while (rc < 0);

  printf("Initialized\n");
  uint8_t test_data[20];

  while(1) {
      delay_ms(1000);
      const char* url = "httpbin.org/post";

      int result = simple_octetstream_post(url, test_data, 20);
      if(result == 200) {
        printf("Append successful\n");
      } else {
        printf("Append failed\n");
      }

      delay_ms(5000);
  }



}
