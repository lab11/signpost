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
#include "simple_post.h"
#include "timer.h"


int main (void) {
  putstr("[GDP_test] ** Main App **\n");

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost
  signpost_initialization_module_init(0x28, NULL);
  putstr("Initialized\n");
  uint8_t test_data[2000];

  while(1) {
      delay_ms(1000);
      const char* url = "gdp.lab11.eecs.umich.edu/gdp/v1/edu.umich.eecs.lab11.signpost-workshop-test/append";

      int result = simple_octetstream_post(url, test_data, 20);
      if(result == 200) {
        putstr("Append successful\n");
      } else {
        putstr("Append failed\n");
      }

      delay_ms(5000);
  }



}
