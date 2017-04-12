#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <random>

#include "led.h"
#include "timer.h"
#include "tock.h"

#include "signpost_api.h"
#include "signbus_app_layer.h"
#include "signbus_io_interface.h"

#ifndef MODULE_ID
#define MODULE_ID 0x46
#endif

#define IV_LENGTH 16

extern "C" {
// This is a bit of a hack
uint8_t* signpost_api_addr_to_key(uint8_t addr);
}

int main(void) {
    printf("\n###\n\n\ni2c blast\n");

    led_on(0);

    int rc;
    do {
        rc = signpost_initialization_module_init(MODULE_ID, SIGNPOST_INITIALIZATION_NO_APIS);
        if (rc < 0) {
            printf(" - Error initializing module (code: %d). Sleeping 5s.\n", rc);
            delay_ms(5000);
        }
    } while (rc < 0);

    uint8_t send_buf[256];
    memset(send_buf, MODULE_ID, 256);

    // Set first IV-length bytes to 0xdd
    memset(send_buf, 0xdd, IV_LENGTH);

    led_off(0);

    // This is a bit hacky, but work around some stability issues a bit by not
    // blasting messages until everyone's initialized
    /*
    for (int i = 0; i < (2 * 10); i++) {
      if (i % 2) {
        led_on(0);
      } else {
        led_off(0);
      }
      delay_ms(500);
    }
    */

    led_on(0);
    printf("BLASTOFF!\n");

    /*
    // Grab a normal distribution to delay
    // http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<> send_distribution(100,20);
    */

    // Actually, a Poisson makes way more sense
    // http://www.cplusplus.com/reference/random/poisson_distribution/
    std::default_random_engine generator;
    std::poisson_distribution<int> send_distribution(100);

    // And payload length
    std::uniform_int_distribution<int> length_distribution(1,100);

    while(1) {
        size_t length = IV_LENGTH + length_distribution(generator);

        rc = signbus_app_send(ModuleAddressController, signpost_api_addr_to_key,
                NotificationFrame, (signbus_api_type_t)0xbe, 0xef, length, send_buf);
        if (rc < 0) {
            printf("Error sending (code: %d)\n", rc);
        }

        // N(100,20) should range roughly 40-160, but just in case
        // P(100) should range roughly 70-130, but just in case
        unsigned delay;
        do {
            delay = send_distribution(generator);
        } while ((delay < 1) || (delay > 200));
        delay_ms(delay);
        //printf("Sent a thing\n");
    }
}
