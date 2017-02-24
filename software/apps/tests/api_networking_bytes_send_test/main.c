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
  printf("[networking_bytes_test] ** Main App **\n");

  int rc;

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost
  do {
    rc = signpost_initialization_module_init(0x27, NULL);
    if (rc < 0) {
      printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
      delay_ms(5000);
    }
  } while (rc < 0);

  printf("Initialized\n");

  while(1) {
      delay_ms(1000);
      const char* test = "This is my networking bytes test data";
      printf("Sending test string to radio module\n");
      int result = signpost_networking_send_bytes(ModuleAddressRadio, (uint8_t*)test, strlen(test));
      if(result > 0) {
        printf("Sent successfully\n");
      } else {
        printf("Send failed\n");
      }

      delay_ms(1000);
      printf("Sending test string to module address 0x30\n");
      result = signpost_networking_send_bytes(0x30, (uint8_t*)test, strlen(test));
      if(result > 0) {
        printf("Sent successfully\n");
      } else {
        printf("Send failed\n");
      }

      delay_ms(5000);
  }



}
