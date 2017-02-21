#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_watchdog.h"
#include "console.h"
#include "controller.h"
#include "fm25cl.h"
#include "gpio_async.h"
#include "gps.h"
#include "i2c_master_slave.h"
#include "minmea.h"
#include "signpost_api.h"
#include "signpost_energy.h"
#include "timer.h"
#include "tock.h"

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
uint8_t  _current_microsecond = 0;
uint32_t _current_latitude = 0;
uint32_t _current_longitude = 0;

static void energy_timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
  get_energy();
}

static void gps_timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
  gps_sample(gps_callback);
}

int mod_isolated_out = -1;
int mod_isolated_in = -1;
int last_mod_isolated_out = -1;
size_t isolated_count = 0;

static void priv_timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
    if(mod_isolated_out < 0) {
        for(size_t i = 0; i < NUM_MOD_IO; i++) {
            if(gpio_read(MOD_OUTS[i]) == 0 && last_mod_isolated_out != MOD_OUTS[i]) {

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
        if(gpio_read(mod_isolated_out) == 1) {
            printf("Module %d done with isolation\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
        }
        // this module took too long to talk to controller
        // XXX need more to police bad modules (repeat offenders)
        else if(isolated_count > 4) {
            printf("Module %d took too long\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            controller_all_modules_enable_i2c();
        }
        else isolated_count++;
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
    //app_watchdog_tickle_kernel();

    gps_tickle = false;
    energy_tickle = false;
  }
}

static void print_energy_data (int module, int energy) {
  char buf[64];
  if (module == 3) {
    snprintf(buf, 64, "  Controller energy: %10i uAh\n", energy);
  } else if (module == 4) {
    snprintf(buf, 64, "  Linux energy:      %10i uAh\n", energy);
  } else {
    snprintf(buf, 64, "  Module %i energy:   %10i uAh\n", module, energy);
  }
  putstr(buf);
}

static void get_energy (void) {
  putstr("\n\nEnergy Data\n");

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
      case 2: print_energy_data(i, fram.energy_module2); break;
      case 3: print_energy_data(i, fram.energy_controller); break;
      case 4: print_energy_data(i, fram.energy_linux); break;
      case 5: print_energy_data(i, fram.energy_module5); break;
      case 6: print_energy_data(i, fram.energy_module6); break;
      case 7: print_energy_data(i, fram.energy_module7); break;
    }
  }

  fm25cl_write_sync(0, sizeof(controller_fram_t));


  uint8_t energy_datum[18];

  // My address
  energy_datum[0] = 0x20;
  // Packet type. 1 == Energy Meters
  energy_datum[1] = 0x01;

  energy_datum[2]  = (uint8_t) (((fram.energy_module0 / 1000) >> 8) & 0xFF);
  energy_datum[3]  = (uint8_t) ((fram.energy_module0 / 1000) & 0xFF);
  energy_datum[4]  = (uint8_t) (((fram.energy_module1 / 1000) >> 8) & 0xFF);
  energy_datum[5]  = (uint8_t) ((fram.energy_module1 / 1000) & 0xFF);
  energy_datum[6]  = (uint8_t) (((fram.energy_module2 / 1000) >> 8) & 0xFF);
  energy_datum[7]  = (uint8_t) ((fram.energy_module2 / 1000) & 0xFF);
  energy_datum[8]  = (uint8_t) (((fram.energy_controller / 1000) >> 8) & 0xFF);
  energy_datum[9]  = (uint8_t) ((fram.energy_controller / 1000) & 0xFF);
  energy_datum[10] = (uint8_t) (((fram.energy_linux / 1000) >> 8) & 0xFF);
  energy_datum[11] = (uint8_t) ((fram.energy_linux / 1000) & 0xFF);
  energy_datum[12] = (uint8_t) (((fram.energy_module5 / 1000) >> 8) & 0xFF);
  energy_datum[13] = (uint8_t) ((fram.energy_module5 / 1000) & 0xFF);
  energy_datum[14] = (uint8_t) (((fram.energy_module6 / 1000) >> 8) & 0xFF);
  energy_datum[15] = (uint8_t) ((fram.energy_module6 / 1000) & 0xFF);
  energy_datum[16] = (uint8_t) (((fram.energy_module7 / 1000) >> 8) & 0xFF);
  energy_datum[17] = (uint8_t) ((fram.energy_module7 / 1000) & 0xFF);

  //make a post url
  const char* url = "gdp.lab11.eecs.umich.edu/gdp/v1/energy_datum/append";
  http_request r;
  //the library will automatically throw in content-length
  //if you don't, because it's annoying to do yourself
  r.num_headers = 1;
  http_header h;
  h.header = "content-type";
  h.value = "application/octet-stream";
  r.headers = &h;
  r.body_len = 18;
  r.body = energy_datum;
  http_response result;

  int rc = signpost_networking_post(url, r, &result);
  if (rc < 0) {
    printf("Failed to do post. return code: %d\n", rc);
  }

  //printf("Energy POST result: %d\n", result);

  // Only say things are working if i2c worked
  if (result.status > 0) {
    // Tickle the watchdog because something good happened.
    //app_watchdog_tickle_kernel();
    watchdog_tickler(2);
  }
}

static void initialization_api_callback(uint8_t source_address,
    signbus_frame_type_t frame_type, signbus_api_type_t api_type,
    uint8_t message_type, __attribute__ ((unused)) size_t message_length,
    uint8_t* message) {
    if (api_type != InitializationApiType) {
      signpost_api_error_reply(source_address, api_type, message_type);
      return;
    }
    int module_number;
    switch (frame_type) {
        case NotificationFrame:
            // XXX unexpected, drop
            break;
        case CommandFrame:
            switch (message_type) {
                case InitializationDeclare:
                    // only if we have a module isolated
                    if (mod_isolated_out < 0) return;
                    module_number = MODOUT_pin_to_mod_name(mod_isolated_out);
                    signpost_initialization_declare_respond(source_address, module_number);
                    break;
                case InitializationKeyExchange:
                    // Prepare and reply ECDH key exchange
                    signpost_initialization_key_exchange_respond(source_address,
                            message, message_length);
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
    signpost_api_error_reply(source_address, api_type, message_type);
    return;
  }

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

      signpost_energy_query_reply(source_address, &info);
    } else if (message_type == EnergyLevelWarning24hMessage) {
      signpost_api_error_reply(source_address, api_type, message_type);
    } else if (message_type == EnergyLevelCritical24hMessage) {
      signpost_api_error_reply(source_address, api_type, message_type);
    } else if (message_type == EnergyCurrentWarning60sMessage) {
      signpost_api_error_reply(source_address, api_type, message_type);
    } else {
      signpost_api_error_reply(source_address, api_type, message_type);
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
  if (api_type != TimeLocationApiType) {
    signpost_api_error_reply(source_address, api_type, message_type);
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
      signpost_timelocation_get_time_reply(source_address, &time);

    } else if (message_type == TimeLocationGetLocationMessage) {
      signpost_timelocation_location_t location;
      location.latitude = _current_latitude;
      location.longitude = _current_longitude;
      signpost_timelocation_get_location_reply(source_address, &location);
    }
  } else if (frame_type == ResponseFrame) {
    // XXX unexpected, drop
  } else if (frame_type == ErrorFrame) {
    // XXX unexpected, drop
  }
}

static void gps_callback (gps_data_t* gps_data) {
  // Got new gps data

  printf("\n\nGPS Data: %d:%d:%d.%lu %d/%d/%d\n",
          gps_data->hours, gps_data->minutes, gps_data->seconds, gps_data->microseconds,
          gps_data->month, gps_data->day, gps_data->year
          );

  printf("  Latitude:   %lu degrees\n", gps_data->latitude);
  printf("  Longitude:  %lu degrees\n", gps_data->longitude);

  const char* fix_str = "Invalid fix";
  if (gps_data->fix == 2) {
      fix_str = "2D fix";
  } else if (gps_data->fix == 3) {
      fix_str = "3D fix";
  }
  printf("  Status:     %s\n", fix_str);
  printf("  Satellites: %d\n", gps_data->satellite_count);


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

  uint8_t gps_datum[18];

  // My address
  gps_datum[0] = 0x20;
  // Packet type. 2 == GPS
  gps_datum[1] = 0x02;
  // date
  gps_datum[2] = (uint8_t) (gps_data->day   & 0xFF);
  gps_datum[3] = (uint8_t) (gps_data->month & 0xFF);
  gps_datum[4] = (uint8_t) (gps_data->year  & 0xFF);
  // time
  gps_datum[5] = (uint8_t) (gps_data->hours   & 0xFF);
  gps_datum[6] = (uint8_t) (gps_data->minutes & 0xFF);
  gps_datum[7] = (uint8_t) (gps_data->seconds & 0xFF);
  // latitude
  gps_datum[8]  = (uint8_t) ((gps_data->latitude >> 24) & 0xFF);
  gps_datum[9]  = (uint8_t) ((gps_data->latitude >> 16) & 0xFF);
  gps_datum[10] = (uint8_t) ((gps_data->latitude >>  8) & 0xFF);
  gps_datum[11] = (uint8_t) ((gps_data->latitude)       & 0xFF);
  // longitude
  gps_datum[12] = (uint8_t) ((gps_data->longitude >> 24) & 0xFF);
  gps_datum[13] = (uint8_t) ((gps_data->longitude >> 16) & 0xFF);
  gps_datum[14] = (uint8_t) ((gps_data->longitude >>  8) & 0xFF);
  gps_datum[15] = (uint8_t) ((gps_data->longitude)       & 0xFF);
  // quality
  gps_datum[16] = (uint8_t) (gps_data->fix & 0xFF);
  gps_datum[17] = (uint8_t) (gps_data->satellite_count & 0xFF);

  //make a post url
  const char* url = "gdp.lab11.eecs.umich.edu/gdp/v1/gps_datum/append";
  http_request r;
  //the library will automatically throw in content-length
  //if you don't, because it's annoying to do yourself
  r.num_headers = 1;
  http_header h;
  h.header = "content-type";
  h.value = "application/octet-stream";
  r.headers = &h;
  r.body_len = 18;
  r.body = gps_datum;

  http_response result;
  int rc = signpost_networking_post(url, r, &result);
  if (rc < 0) {
    printf("Failed to do GPS post. return code: %d\n", rc);
  }

  //printf("GPS POST result: %d\n", result);

  // Only say things are working if i2c worked
  // this means we got a post response- that's a win
  if (result.status > 0) {
    // Tickle the watchdog because something good happened.
    //app_watchdog_tickle_kernel();
    watchdog_tickler(1);
  }
}

int main (void) {
  putstr("[Controller] ** Main App **\n");

  ///////////////////
  // Local Operations
  // ================
  //
  // Initializations that only touch the controller board

  // Configure FRAM
  // --------------
  putstr("Configuring FRAM\n");
  fm25cl_set_read_buffer((uint8_t*) &fram, sizeof(controller_fram_t));
  fm25cl_set_write_buffer((uint8_t*) &fram, sizeof(controller_fram_t));

  // Read FRAM to see if anything is stored there
  const unsigned FRAM_MAGIC_VALUE = 0x49A80003;
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
  putstr("GPS\n");
  gps_init();


  // Timers
  // ------
  putstr("Starting timers\n");
  timer_subscribe(energy_timer_callback, NULL);
  bonus_timer_subscribe(gps_timer_callback, NULL);

  // Start timers to read energy and GPS
  timer_start_repeating(10000);
  bonus_timer_start_repeating(27000);



  //////////////////////////////////////
  // Remote System Management Operations
  //
  // Initializations that over the system management bus that shouldn't affect
  // signpost modules

  // Energy Management
  // -----------------

  // Configure all the I2C selectors
  putstr("Init'ing energy\n");
  signpost_energy_init();

  // Reset all of the LTC2941s
  putstr("Resetting energy\n");
  signpost_energy_reset();



  /////////////////////////////
  // Signpost Module Operations
  //
  // Initializations for the rest of the signpost

  // Install hooks for the signpost APIs we implement
  static api_handler_t init_handler   = {InitializationApiType, initialization_api_callback};
  static api_handler_t energy_handler = {EnergyApiType, energy_api_callback};
  static api_handler_t timelocation_handler = {TimeLocationApiType, timelocation_api_callback};
  static api_handler_t* handlers[] = {&init_handler, &energy_handler, &timelocation_handler, NULL};
  signpost_initialization_controller_module_init(handlers);

  // Setup backplane by enabling the modules
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();
  controller_all_modules_disable_usb();
  controller_gpio_enable_all_MODINs();
  controller_gpio_enable_all_MODOUTs(PullUp);
  controller_gpio_set_all();
  // controller_all_modules_disable_i2c();
  // controller_module_enable_i2c(MODULE5);
  // controller_module_enable_i2c(MODULE0);




  ////////////////////////////////////////////////
  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("Everything intialized\n");

  putstr("Entering loop\n");
  while(1) {
    delay_ms(2000);
    priv_timer_callback(0, 0, 0, NULL);
  }
}
