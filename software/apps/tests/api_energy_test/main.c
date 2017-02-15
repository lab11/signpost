#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "console.h"
#include "signpost_api.h"
#include "timer.h"
#include "tock.h"

int counter = 0;

/*
static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
}
*/

static const uint8_t i2c_address = 0x50;

int main (void) {
  putstr("[Test] API: Energy\n");

  signpost_initialization_module_init(
      i2c_address,
      SIGNPOST_INITIALIZATION_NO_APIS);

  int rc;
  signpost_energy_information_t info;

  while (true) {
    printf("\nQuery Energy\n");
    printf(  "============\n\n");
    rc = signpost_energy_query(&info);
    if (rc < 0) {
      printf("Error querying energy: %d\n\n", rc);
    } else {
      printf("Energy Query Result:\n");
      printf("      24h limit: %-4lu mJ\n", info.energy_limit_24h_mJ);
      printf("      24h used:  %-4lu mJ\n", info.energy_used_24h_mJ);
      printf("      60s limit: %-4u mA\n", info.current_limit_60s_mA);
      printf("    60s average: %-4u mA\n", info.current_average_60s_mA);
      printf("  mJ limit warn: %-4u%%\n",   info.energy_limit_warning_threshold);
      printf("  mJ limit crit: %-4u%%\n",   info.energy_limit_critical_threshold);
      printf("\n");
    }

    printf("Sleeping for 5s\n");
    delay_ms(5000);
  }

  /*
  // Need a timer
  timer_subscribe(timer_callback, NULL);
  timer_start_repeating(750);
  */
}
