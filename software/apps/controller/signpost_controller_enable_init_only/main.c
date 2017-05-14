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
#include "controller.h"
#include "gpio_async.h"
#include "signpost_api.h"
#include "signpost_energy.h"

int mod_isolated_out = -1;
int mod_isolated_in = -1;
int last_mod_isolated_out = -1;
size_t isolated_count = 0;

static void check_module_initialization (void) {
    if (mod_isolated_out < 0) {
        for (size_t i = 0; i < NUM_MOD_IO; i++) {
            if (gpio_read(MOD_OUTS[i]) == 0 && last_mod_isolated_out != MOD_OUTS[i]) {

                printf("Module %d granted isolation\n", MODOUT_pin_to_mod_name(MOD_OUTS[i]));
                // module requesting isolation
                mod_isolated_out = MOD_OUTS[i];
                mod_isolated_in = MOD_INS[i];
                last_mod_isolated_out = MOD_OUTS[i];
                isolated_count = 0;

                // create private channel for this module
                //XXX warn modules of i2c disable
                controller_all_modules_disable_i2c();
                controller_module_enable_i2c(MODOUT_pin_to_mod_name(mod_isolated_out));
                // signal to module that it has a private channel
                // XXX this should be a controller function operating on the
                // module number, not index
                gpio_clear(mod_isolated_in);
                delay_ms(1000);
                break;
            }
            // didn't isolate anyone, reset last_mod_isolated_out
            last_mod_isolated_out = -1;
        }
    } else {
        if (gpio_read(mod_isolated_out) == 1) {
            printf("Module %d done with isolation\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
        }
        // this module took too long to talk to controller
        // XXX need more to police bad modules (repeat offenders)
        else if (isolated_count > 15) {
            printf("Module %d took too long\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
        } else {
          isolated_count++;
        }
    }
}

static void initialization_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, __attribute__ ((unused)) size_t message_length,
    uint8_t* message) {
    if (api_type != InitializationApiType) {
      signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
      return;
    }
    int module_number;
    int rc;
    switch (frame_type) {
        case NotificationFrame:
            // XXX unexpected, drop
            break;
        case CommandFrame:
            switch (message_type) {
                case InitializationDeclare:
                    // only if we have a module isolated or from storage master
                    if (mod_isolated_out < 0 && source_address != ModuleAddressStorage) return;
                    if (source_address == ModuleAddressStorage) module_number = 4;
                    else module_number = MODOUT_pin_to_mod_name(mod_isolated_out);
                    rc = signpost_initialization_declare_respond(source_address, module_number);
                    if (rc < 0) {
                      printf(" - %d: Error responding to initialization declare request for module %d at address 0x%02x. Dropping.\n",
                          __LINE__, module_number, source_address);
                    }
                    break;
                case InitializationKeyExchange:
                    // Prepare and reply ECDH key exchange
                    rc = signpost_initialization_key_exchange_respond(source_address,
                            message, message_length);
                    if (rc < 0) {
                      printf(" - %d: Error responding to key exchange at address 0x%02x. Dropping.\n",
                          __LINE__, source_address);
                    }
                    break;
                //exchange module
                //get mods
                default:
                   break;
            }
        case ResponseFrame:
            // XXX unexpected, drop
            break;
        case ErrorFrame:
            // XXX unexpected, drop
            break;
        default:
            break;
    }
}


int main (void) {
  printf("[Controller] ** Enable & Init Only **\n");

  int rc;

  ///////////////////
  // Local Operations
  // ================
  //
  // Initializations that only touch the controller board

  /* None */

  //////////////////////////////////////
  // Remote System Management Operations
  //
  // Initializations that over the system management bus that shouldn't affect
  // signpost modules

  /* None */

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost

  // Install hooks for the signpost APIs we implement
  static api_handler_t init_handler   = {InitializationApiType, initialization_api_callback};
  static api_handler_t* handlers[] = {&init_handler, NULL};
  do {
    rc = signpost_initialization_controller_module_init(handlers);
    if (rc < 0) {
      printf(" - Error initializing as controller module with signpost library (code: %d)\n", rc);
      printf("   Sleeping 5s\n");
      delay_ms(5000);
    }
  } while (rc < 0);

  // Setup backplane by enabling the modules
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();
  controller_all_modules_disable_usb();
  controller_gpio_enable_all_MODINs();
  controller_gpio_enable_all_MODOUTs(PullUp);
  controller_gpio_set_all();


  ////////////////////////////////////////////////
  // Setup watchdog
  app_watchdog_set_kernel_timeout(10000);
  app_watchdog_start();

  printf("Everything intialized\n");

  delay_ms(500);

  printf("Entering loop\n");
  while(1) {
    // always check for modules that need to be initialized
    check_module_initialization();

    delay_ms(100);

    app_watchdog_tickle_kernel();
  }
}

