#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <i2c_master_slave.h>
#include <timer.h>
#include <tock.h>

#include "app_watchdog.h"
#include "controller.h"
#include "fm25cl.h"
#include "gpio_async.h"
#include "gps.h"
#include "minmea.h"
#include "signpost_api.h"
#include "signpost_energy.h"

#include "bonus_timer.h"

static void get_energy (void);
static void gps_callback (gps_data_t* gps_data);

uint8_t fm25cl_read_buf[256];
uint8_t fm25cl_write_buf[256];

uint16_t _current_year = 0;
uint8_t  _current_month = 0;
uint8_t  _current_day = 0;
uint8_t  _current_hour = 0;
uint8_t  _current_minute = 0;
uint8_t  _current_second = 0;
uint8_t  _current_satellite_count = 0;
uint8_t  _current_microsecond = 0;
uint32_t _current_latitude = 0;
uint32_t _current_longitude = 0;

int mod_isolated_out = -1;
int mod_isolated_in = -1;
int last_mod_isolated_out = -1;
size_t isolated_count = 0;
uint8_t currently_initializing;
uint8_t module_init_failures[8] = {0};
uint8_t watchdog_tickled[8] = {0};
uint8_t watchdog_subscribed[8] = {0};
uint8_t module_addresses[8] = {0};

static void check_module_initialization (void) {
    if (mod_isolated_out < 0) {
        for (size_t i = 0; i < NUM_MOD_IO; i++) {
            if (gpio_read(MOD_OUTS[i]) == 0 && last_mod_isolated_out != MOD_OUTS[i]) {

                printf("Module %d granted isolation\n", MODOUT_pin_to_mod_name(MOD_OUTS[i]));
                currently_initializing = 1;
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
            currently_initializing = 0;
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
            module_init_failures[MODOUT_pin_to_mod_name(mod_isolated_out)] = 0;
        }
        // this module took too long to talk to controller
        // XXX need more to police bad modules (repeat offenders)
        else if (isolated_count > 15) {
            printf("Module %d took too long\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            currently_initializing = 0;
            gpio_set(mod_isolated_in);
            module_init_failures[MODOUT_pin_to_mod_name(mod_isolated_out)]++;
            if(module_init_failures[MODOUT_pin_to_mod_name(mod_isolated_out)] > 4) {
                //power cycle the module
                controller_module_disable_power(MODOUT_pin_to_mod_name(mod_isolated_out));
                delay_ms(1000);
                controller_module_enable_power(MODOUT_pin_to_mod_name(mod_isolated_out));
                module_init_failures[MODOUT_pin_to_mod_name(mod_isolated_out)] = 0;
            }
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
        } else {
          isolated_count++;
        }
    }
}


typedef struct {
  uint32_t magic;
  uint32_t energy_controller;
  uint32_t energy_linux;
  uint32_t energy_module0;
  uint32_t energy_module1;
  uint32_t energy_module2;
  uint32_t energy_module5;
  uint32_t energy_module6;
  uint32_t energy_module7;
} controller_fram_t;

// Keep track of the last time we got data from the ltc chips
// so we can do diffs when reading the energy.
uint32_t energy_last_readings[128] = {0};

controller_fram_t fram;

static void watchdog_tickler (int which) {
  static bool gps_tickle = false;
  static bool energy_tickle = false;

  if (which == 1) {
    gps_tickle = true;
  } else {
    energy_tickle = true;
  }

  if (gps_tickle && energy_tickle) {
    app_watchdog_tickle_kernel();

    gps_tickle = false;
    energy_tickle = false;
  }
}

static void print_energy_data (int module, int energy) {
  if (module == 3) {
    printf("  Controller energy: %10i uAh\n", energy);
  } else if (module == 4) {
    printf("  Linux energy:      %10i uAh\n", energy);
  } else {
    printf("  Module %i energy:   %10i uAh\n", module, energy);
  }
}

static void get_energy (void) {
  printf("\n\nEnergy Data\n");

  for (int i=0; i<8; i++) {
    uint32_t energy;
    uint32_t* last_reading = &energy_last_readings[i];

    if (i == 3) {
      energy = signpost_ltc_to_uAh(signpost_energy_get_controller_energy(), POWER_MODULE_RSENSE, POWER_MODULE_PRESCALER);
    } else if (i == 4) {
      energy = signpost_ltc_to_uAh(signpost_energy_get_linux_energy(), POWER_MODULE_RSENSE, POWER_MODULE_PRESCALER);
    } else {
      energy = signpost_ltc_to_uAh(signpost_energy_get_module_energy(i), POWER_MODULE_RSENSE, POWER_MODULE_PRESCALER);
    }

    uint32_t diff = energy - *last_reading;
    *last_reading = energy;

    switch (i) {
      case 0: fram.energy_module0 += diff; break;
      case 1: fram.energy_module1 += diff; break;
      case 2: fram.energy_module2 += diff; break;
      case 3: fram.energy_controller += diff; break;
      case 4: fram.energy_linux += diff; break;
      case 5: fram.energy_module5 += diff; break;
      case 6: fram.energy_module6 += diff; break;
      case 7: fram.energy_module7 += diff; break;
    }

    // Test print
    switch (i) {
      case 0: print_energy_data(i, fram.energy_module0); break;
      case 1: print_energy_data(i, fram.energy_module1); break;
      //case 2: print_energy_data(i, fram.energy_module2); break;
      case 3: print_energy_data(i, fram.energy_controller); break;
      case 4: print_energy_data(i, fram.energy_linux); break;
      //case 5: print_energy_data(i, fram.energy_module5); break;
      //case 6: print_energy_data(i, fram.energy_module6); break;
      //case 7: print_energy_data(i, fram.energy_module7); break;
    }
  }

  fm25cl_write_sync(0, sizeof(controller_fram_t));

  //send this data to the radio module
  static uint8_t send_buf[17];
  send_buf[0] = 0x01;
  send_buf[1] = ((fram.energy_module0 & 0xFF00) >> 8 );
  send_buf[2] = ((fram.energy_module0 & 0xFF));
  send_buf[3] = ((fram.energy_module1 & 0xFF00) >> 8 );
  send_buf[4] = ((fram.energy_module1 & 0xFF));
  send_buf[5] = ((fram.energy_module2 & 0xFF00) >> 8 );
  send_buf[6] = ((fram.energy_module2 & 0xFF));
  send_buf[7] = ((fram.energy_controller & 0xFF00) >> 8 );
  send_buf[8] = ((fram.energy_controller & 0xFF));
  send_buf[9] = ((fram.energy_linux & 0xFF00) >> 8 );
  send_buf[10] = ((fram.energy_linux & 0xFF));
  send_buf[11] = ((fram.energy_module5 & 0xFF00) >> 8 );
  send_buf[12] = ((fram.energy_module5 & 0xFF));
  send_buf[13] = ((fram.energy_module6 & 0xFF00) >> 8 );
  send_buf[14] = ((fram.energy_module6 & 0xFF));
  send_buf[15] = ((fram.energy_module7 & 0xFF00) >> 8 );
  send_buf[16] = ((fram.energy_module7 & 0xFF));

  int rc;
  if(!currently_initializing) {
    rc = signpost_networking_send_bytes(ModuleAddressRadio,send_buf,17);
  } else {
    rc = 0;
  }

  if(rc >= 0) {
    // Tickle the watchdog because something good happened.
  }

  watchdog_tickler(2);

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
                    if (mod_isolated_out < 0 && source_address != ModuleAddressStorage) {
                        return;
                    }
                    if (source_address == ModuleAddressStorage)  {
                        module_number = 4;
                    }
                    else {
                        module_number = MODOUT_pin_to_mod_name(mod_isolated_out);
                    }

                    module_addresses[module_number] = source_address;
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

static void energy_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, __attribute__ ((unused)) size_t message_length, __attribute__ ((unused)) uint8_t* message) {
  if (api_type != EnergyApiType) {
    signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    return;
  }

  int rc;

  if (frame_type == NotificationFrame) {
    // XXX unexpected, drop
  } else if (frame_type == CommandFrame) {
    if (message_type == EnergyQueryMessage) {
      signpost_energy_information_t info;
      info.energy_limit_24h_mJ = 1;
      info.energy_used_24h_mJ = 2;
      info.current_limit_60s_mA = 3;
      info.current_average_60s_mA = 4;
      info.energy_limit_warning_threshold = 5;
      info.energy_limit_critical_threshold = 6;

      rc = signpost_energy_query_reply(source_address, &info);
      if (rc < 0) {
        printf(" - %d: Error sending energy query reply (code: %d). Replying with fail.\n", __LINE__, rc);
        signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
      }
    } else if (message_type == EnergyLevelWarning24hMessage) {
      signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    } else if (message_type == EnergyLevelCritical24hMessage) {
      signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    } else if (message_type == EnergyCurrentWarning60sMessage) {
      signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    } else {
      signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    }
  } else if (frame_type == ResponseFrame) {
    // XXX unexpected, drop
  } else if (frame_type == ErrorFrame) {
    // XXX unexpected, drop
  }
}

// Callback for when a different module requests time or location information.
static void timelocation_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, __attribute__ ((unused)) size_t message_length, __attribute__ ((unused)) uint8_t* message) {
  int rc;

  if (api_type != TimeLocationApiType) {
    signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    return;
  }

  if (frame_type == NotificationFrame) {
    // XXX unexpected, drop
  } else if (frame_type == CommandFrame) {
    if (message_type == TimeLocationGetTimeMessage) {
      // Copy in time data from the most recent time we got an update
      // from the GPS.
      signpost_timelocation_time_t time;
      time.year = _current_year;
      time.month = _current_month;
      time.day = _current_day;
      time.hours = _current_hour;
      time.minutes = _current_minute;
      time.seconds = _current_second;
      time.satellite_count = _current_satellite_count;
      rc = signpost_timelocation_get_time_reply(source_address, &time);
      if (rc < 0) {
        printf(" - %d: Error sending TimeLocationGetTimeMessage reply (code: %d).\n", __LINE__, rc);
        signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
      }

    } else if (message_type == TimeLocationGetLocationMessage) {
      signpost_timelocation_location_t location;
      location.latitude = _current_latitude;
      location.longitude = _current_longitude;
      location.satellite_count = _current_satellite_count;
      rc = signpost_timelocation_get_location_reply(source_address, &location);
      if (rc < 0) {
        printf(" - %d: Error sending TimeLocationGetLocationMessage reply (code: %d).\n", __LINE__, rc);
        signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
      }
    }
  } else if (frame_type == ResponseFrame) {
    // XXX unexpected, drop
  } else if (frame_type == ErrorFrame) {
    // XXX unexpected, drop
  }
}

static void check_watchdogs(void) {
    for(uint8_t j = 0; j < 8; j++) {
        if(watchdog_subscribed[j] != 0) {
            if(watchdog_tickled[j] == 0) {
                for(uint8_t i = 0; i < 8; j++) {
                    if(module_addresses[i] == watchdog_subscribed[j]) {
                        printf("Watchdog Timeout %d\n",watchdog_subscribed[j]);
                        controller_module_disable_power(i);
                        delay_ms(300);
                        controller_module_enable_power(i);
                    }
                    break;
                }
            } else {
                printf("Watchdog good %d\n",watchdog_subscribed[j]);
                watchdog_tickled[j] = 0;
            }
        }
    }
}

static void watchdog_api_callback(uint8_t source_address,
    __attribute__ ((unused)) signbus_frame_type_t frame_type, __attribute__ ((unused)) signbus_api_type_t api_type,
    uint8_t message_type, __attribute__ ((unused)) size_t message_length, __attribute__ ((unused)) uint8_t* message) {

    printf("Got watchdog API call\n");

    if(message_type == WatchdogStartMessage) {
        int rc = signpost_watchdog_reply(source_address);
        if(rc >= 0) {
            for(uint8_t j = 0; j < 8; j++) {
                if(watchdog_subscribed[j] == 0 || watchdog_subscribed[j] == source_address) {
                    printf("Subscribed Watchdog %d\n",source_address);
                    watchdog_subscribed[j] = source_address;

                    //give them one tickle
                    watchdog_tickled[j] = 1;
                    break;
                }
            }
        }
    } else if(message_type == WatchdogTickleMessage)  {
        int rc = signpost_watchdog_reply(source_address);
        if(rc >= 0) {
            for(uint8_t j = 0; j < 8; j++) {
                if(watchdog_subscribed[j] == source_address) {
                    printf("Tickled Watchdog %d\n",source_address);
                    watchdog_tickled[j] = 1;
                    break;
                }
            }
        }
    }
}

static void gps_callback (gps_data_t* gps_data) {
  // Got new gps data

  /*printf("\n\nGPS Data: %d:%02d:%02d.%lu %d/%d/%d\n",
          gps_data->hours, gps_data->minutes, gps_data->seconds, gps_data->microseconds,
          gps_data->month, gps_data->day, gps_data->year
          );

  printf("  Latitude:   %lu degrees\n", gps_data->latitude);
  printf("  Longitude:  %lu degrees\n", gps_data->longitude);*/

  const char* fix_str = "Invalid fix";
  if (gps_data->fix == 2) {
      fix_str = "2D fix";
  } else if (gps_data->fix == 3) {
      fix_str = "3D fix";
  }
  //printf("  Status:     %s\n", fix_str);
  //printf("  Satellites: %d\n", gps_data->satellite_count);


  // Save most recent GPS reading for anyone that wants location and time.
  // This isn't a great method for proving the TimeLocation API, but it's
  // pretty easy to do. We eventually need to reduce the sleep current of
  // the controller, which will mean better handling of GPS.
  _current_year = gps_data->year + 2000;
  _current_month = gps_data->month;
  _current_day = gps_data->day;
  _current_hour = gps_data->hours;
  _current_minute = gps_data->minutes;
  _current_second = gps_data->seconds;
  _current_microsecond = gps_data->microseconds;
  _current_latitude = gps_data->latitude;
  _current_longitude = gps_data->longitude;
  _current_satellite_count = gps_data->satellite_count;

  //send a gps reading to the radio so that it can transmit it
  int rc;
  if(!currently_initializing) {
    static uint8_t send_buf[17];
    send_buf[0] = 0x02; //gps type
    send_buf[1] = _current_day;
    send_buf[2] = _current_month;
    send_buf[3] = _current_year;
    send_buf[4] = _current_hour;
    send_buf[5] = _current_minute;
    send_buf[6] = _current_second;
    send_buf[7] = ((_current_latitude & 0xFF000000) >> 24);
    send_buf[8] = ((_current_latitude & 0x00FF0000) >> 16);
    send_buf[9] = ((_current_latitude & 0x0000FF00) >> 8);
    send_buf[10] = ((_current_latitude & 0x000000FF));
    send_buf[11] = ((_current_longitude & 0xFF000000) >> 24);
    send_buf[12] = ((_current_longitude & 0x00FF0000) >> 16);
    send_buf[13] = ((_current_longitude & 0x0000FF00) >> 8);
    send_buf[14] = ((_current_longitude & 0x000000FF));
    if(_current_satellite_count >= 3) {
      send_buf[15] = 0x02;
    } else if (_current_satellite_count >=4) {
      send_buf[15] = 0x03;
    } else {
      send_buf[15] = 0x01;
    }
    send_buf[16] = _current_satellite_count;
    rc = signpost_networking_send_bytes(ModuleAddressRadio,send_buf,17);
  } else {
      rc = 0;
  }

  // Tickle the watchdog because something good happened.
  //app_watchdog_tickle_kernel();
  if (rc >= 0) {

  }

  watchdog_tickler(1);

  // start sampling again to catch the next second
  gps_sample(gps_callback);
}

int main (void) {
  printf("[Controller] ** Main App **\n");

  int rc;

  ///////////////////
  // Local Operations
  // ================
  //
  // Initializations that only touch the controller board

  // Configure FRAM
  // --------------
  printf("Configuring FRAM\n");
  fm25cl_set_read_buffer((uint8_t*) &fram, sizeof(controller_fram_t));
  fm25cl_set_write_buffer((uint8_t*) &fram, sizeof(controller_fram_t));

  // Read FRAM to see if anything is stored there
  const unsigned FRAM_MAGIC_VALUE = 0x49A80004;
  fm25cl_read_sync(0, sizeof(controller_fram_t));
  if (fram.magic == FRAM_MAGIC_VALUE) {
    // Great. We have saved data.
  } else {
    // Initialize this
    fram.magic = FRAM_MAGIC_VALUE;
    fram.energy_controller = 0;
    fram.energy_linux = 0;
    fram.energy_module0 = 0;
    fram.energy_module1 = 0;
    fram.energy_module2 = 0;
    fram.energy_module5 = 0;
    fram.energy_module6 = 0;
    fram.energy_module7 = 0;
    fm25cl_write_sync(0, sizeof(controller_fram_t));
  }

  // Setup GPS
  // ---------
  printf("GPS\n");
  gps_init();
  gps_sample(gps_callback);

  //////////////////////////////////////
  // Remote System Management Operations
  //
  // Initializations that over the system management bus that shouldn't affect
  // signpost modules

  // Energy Management
  // -----------------

  // Configure all the I2C selectors
  printf("Init'ing energy\n");
  signpost_energy_init();

  // Reset all of the LTC2941s
  printf("Resetting energy\n");
  signpost_energy_reset();

  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost

  // Install hooks for the signpost APIs we implement
  static api_handler_t init_handler   = {InitializationApiType, initialization_api_callback};
  static api_handler_t energy_handler = {EnergyApiType, energy_api_callback};
  static api_handler_t timelocation_handler = {TimeLocationApiType, timelocation_api_callback};
  static api_handler_t watchdog_handler = {WatchdogApiType, watchdog_api_callback};
  static api_handler_t* handlers[] = {&init_handler, &energy_handler, &timelocation_handler, &watchdog_handler, NULL};
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
  app_watchdog_set_kernel_timeout(30000);
  app_watchdog_start();

  printf("Everything intialized\n");

  delay_ms(2000);
  // hack: initialize radio first
  size_t j = 2;
  printf("Module %d granted isolation\n", MODOUT_pin_to_mod_name(MOD_OUTS[j]));
  // module requesting isolation
  mod_isolated_out = MOD_OUTS[j];
  printf("%d\n", mod_isolated_out);
  mod_isolated_in = MOD_INS[j];
  printf("%d\n", mod_isolated_in);
  last_mod_isolated_out = MOD_OUTS[j];
  isolated_count = 0;

  // create private channel for this module
  //XXX warn modules of i2c disable
  controller_all_modules_disable_i2c();
  controller_module_enable_i2c(MODOUT_pin_to_mod_name(mod_isolated_out));
  // signal to module that it has a private channel
  // XXX this should be a controller function operating on the
  // module number, not index
  gpio_clear(mod_isolated_in);

  printf("Entering loop\n");
  uint8_t index = 0;
  while(1) {
    // always check for modules that need to be initialized
    check_module_initialization();

    // get energy updates every 10 seconds
    if ((index % 10) == 0) {
      printf("Check energy\n");
      get_energy();

    }

    if ((index % 50) == 0 && index != 0) {
        if(mod_isolated_out < 0) {
            check_watchdogs();
        }
    }

    index++;
    delay_ms(1000);
  }
}

