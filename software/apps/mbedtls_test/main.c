
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <tock.h>
#include "firestorm.h"
#include "gpio.h"
#include "adc.h"
#include "app_watchdog.h"
#include "msgeq7.h"
#include "i2c_master_slave.h"
#include "timer.h"
#include "mbedtls/md.h"

#define PIN 4

int main(void) {
    gpio_enable_output(PIN);
    gpio_clear(PIN);

    printf("Test mbedtls\n");

    unsigned char test[] = "blahblahblahblahblahblahblahblah";
    unsigned char key[] = "secret";
    char output[32];
    mbedtls_md_info_t * md_info;
    mbedtls_md_context_t md_context;

    while(1) {
      gpio_set(PIN);
      // get parameters for hash
      md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
      // clear context
      mbedtls_md_free(&md_context);
      // init context
      mbedtls_md_init(&md_context);
      // setup context
      mbedtls_md_setup(&md_context, md_info, 1);
      // start digest
      mbedtls_md_hmac_starts(&md_context, key, 6);
      // update digest
      mbedtls_md_hmac_update(&md_context, test, 32);
      // finish
      mbedtls_md_hmac_finish(&md_context, output);

      gpio_clear(PIN);

      delay_ms(500);

      printf("output: 0x");
      for(int i = 0; i < 32; i++) {
        printf(" %x ", output[i]);
      }
      printf("\n");

    }
}
