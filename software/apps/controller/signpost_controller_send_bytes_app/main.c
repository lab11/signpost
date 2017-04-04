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

static void get_energy_remaining (void);
static void get_energy_average (void);
static void get_batsol (void);
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

uint8_t gps_buf[20];
uint8_t energy_buf[20];
uint8_t energy_av_buf[20];
uint8_t batsol_buf[20];
int32_t module_duty_cycle[8] = {-1};
uint8_t num_inited = 0;
uint8_t module_disabled[8] = {0};

static void enable_all_enabled_i2c(void) {
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) continue;
        if(module_disabled[i] == 0 && module_duty_cycle[i] == -1) {
            controller_module_enable_i2c(i);
        }
    }
}

static void enable_all_enabled_power(void) {
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) continue;
        if(module_disabled[i] == 0 && module_duty_cycle[i] == -1) {
            controller_module_enable_power(i);
        }
    }
}

static void check_module_energy_remaining(void) {
    //now after updating energy we should disable the modules that
    //have used too much energy - also add them to a disabled list
    for(uint8_t i = 0; i < 8; i++) {
        if(i==3 || i == 4 || i == 1) continue;

        if(signpost_energy_get_module_energy_remaining(i) < 0) {
            module_disabled[i] = 1;
            controller_module_disable_power(i);
            controller_module_disable_i2c(i);
        } else {
            //turn them back on only if they had used too much power
            //we don't want to turn on modules being duty cycled
            if(module_disabled[i] == 1) {
                module_disabled[i] = 0;
                controller_module_enable_power(i);
                controller_module_enable_i2c(i);
            }
        }
    }
}

static void check_module_duty_cycle(void) {
    for(uint8_t i = 0; i < 8; i++) {
        if(module_duty_cycle[i] == 0) {
            //turn them back on - only if they haven't used too much pwoer
            if(module_disabled[i] == 0) {
                controller_module_enable_power(i);
            }

            //set it back to -1
            module_duty_cycle[i] = -1;
        } else if(module_duty_cycle[i] > 0) {
            module_duty_cycle[i]--;
        }
    }
}

static void check_module_initialization (void) {
    if (mod_isolated_out < 0) {
        for (size_t i = 0; i < NUM_MOD_IO; i++) {
            if (gpio_read(MOD_OUTS[i]) == 0 && last_mod_isolated_out != MOD_OUTS[i]) {

                printf("ISOLATION: Module %d granted isolation\n", MODOUT_pin_to_mod_name(MOD_OUTS[i]));
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
            printf("ISOLATION: Module %d done with isolation\n", MODOUT_pin_to_mod_name(mod_isolated_out));
            currently_initializing = 0;
            gpio_set(mod_isolated_in);
            mod_isolated_out = -1;
            mod_isolated_in  = -1;
            enable_all_enabled_i2c();
            module_init_failures[MODOUT_pin_to_mod_name(mod_isolated_out)] = 0;
        }
        // this module took too long to talk to controller
        // XXX need more to police bad modules (repeat offenders)
        else if (isolated_count > 15) {
            printf("ISOLATION: Module %d took too long\n", MODOUT_pin_to_mod_name(mod_isolated_out));
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
            enable_all_enabled_i2c();
        } else {
          isolated_count++;
        }
    }
}


typedef struct {
  uint32_t magic;
  signpost_energy_remaining_t energy;
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


static void get_energy_average (void) {

  int c_power = signpost_energy_get_controller_average_power()/1000;
  if(c_power <= 0) {
      c_power = 0;
  }

  energy_av_buf[8] = (((c_power) & 0xFF00) >> 8 );
  energy_av_buf[9] = (((c_power) & 0xFF));
  energy_av_buf[10] = 0;
  energy_av_buf[11] = 0;

  //send this data to the radio module
  for(uint8_t i = 0; i < 8; i++) {
      if(i==3 || i == 4) continue;

      int mod_energy = signpost_energy_get_module_average_power(i)/1000;

      if(mod_energy <= 0) {
          mod_energy = 0;
      }

      energy_av_buf[2+i*2] = (((mod_energy) & 0xFF00) >> 8);
      energy_av_buf[2+i*2+1] = (((mod_energy) & 0xFF));
  }

  int rc;
  if(!currently_initializing && num_inited > 1) {
    rc = signpost_networking_send_bytes(ModuleAddressRadio,energy_av_buf,18);
    energy_av_buf[1]++;
  } else {
    rc = 0;
  }

  if(rc >= 0) {
    // Tickle the watchdog because something good happened.
  }

  watchdog_tickler(2);

}

static void get_energy_remaining (void) {

  //read the energy remaining data
    fram.energy.controller_energy_remaining = signpost_energy_get_controller_energy_remaining();
    for(uint8_t i = 0; i < 8; i++) {
        if(i == 3 || i == 4) continue;
        fram.energy.module_energy_remaining[i] = signpost_energy_get_module_energy_remaining(i);
    }

  fm25cl_write_sync(0, sizeof(controller_fram_t));

  //send this data to the radio module
  energy_buf[2] = (((fram.energy.module_energy_remaining[0]/1000) & 0xFF00) >> 8 );
  energy_buf[3] = (((fram.energy.module_energy_remaining[0]/1000) & 0xFF));
  energy_buf[4] = (((fram.energy.module_energy_remaining[1]/1000) & 0xFF00) >> 8 );
  energy_buf[5] = (((fram.energy.module_energy_remaining[1]/1000) & 0xFF));
  energy_buf[6] = (((fram.energy.module_energy_remaining[2]/1000) & 0xFF00) >> 8 );
  energy_buf[7] = (((fram.energy.module_energy_remaining[2]/1000) & 0xFF));
  energy_buf[8] = (((fram.energy.controller_energy_remaining/1000) & 0xFF00) >> 8 );
  energy_buf[9] = (((fram.energy.controller_energy_remaining/1000) & 0xFF));
  energy_buf[10] = 0;
  energy_buf[11] = 0;
  energy_buf[12] = (((fram.energy.module_energy_remaining[5]/1000) & 0xFF00) >> 8 );
  energy_buf[13] = (((fram.energy.module_energy_remaining[5]/1000) & 0xFF));
  energy_buf[14] = (((fram.energy.module_energy_remaining[6]/1000) & 0xFF00) >> 8 );
  energy_buf[15] = (((fram.energy.module_energy_remaining[6]/1000) & 0xFF));
  energy_buf[16] = (((fram.energy.module_energy_remaining[7]/1000) & 0xFF00) >> 8 );
  energy_buf[17] = (((fram.energy.module_energy_remaining[7]/1000) & 0xFF));

  printf("/**************************************/\n");
  printf("\tModule 0 energy remaining %d uWh\n",fram.energy.module_energy_remaining[0]);
  printf("\tModule 1 energy remaining %d uWh\n",fram.energy.module_energy_remaining[1]);
  printf("\tModule 2 energy remaining %d uWh\n",fram.energy.module_energy_remaining[2]);
  printf("\tModule 5 energy remaining %d uWh\n",fram.energy.module_energy_remaining[5]);
  printf("\tModule 6 energy remaining %d uWh\n",fram.energy.module_energy_remaining[6]);
  printf("\tModule 7 energy remaining %d uWh\n",fram.energy.module_energy_remaining[7]);
  printf("\tController energy remaining %d uWh\n",fram.energy.controller_energy_remaining);
  printf("/**************************************/\n");

  int rc;
  if(!currently_initializing && num_inited > 1) {
    rc = signpost_networking_send_bytes(ModuleAddressRadio,energy_buf,18);
    energy_buf[1]++;
  } else {
    rc = 0;
  }

  if(rc >= 0) {
    // Tickle the watchdog because something good happened.
  }

  watchdog_tickler(2);

}

static void get_batsol (void) {
  int battery_voltage = signpost_energy_get_battery_voltage();
  int battery_current = signpost_energy_get_battery_current();
  int battery_energy = (int)((signpost_energy_get_battery_energy_remaining()/BATTERY_VOLTAGE_NOM)/1000.0);
  int battery_percent = (int)(signpost_energy_get_battery_percent()/1000.0);
  int battery_full = (int)((signpost_energy_get_battery_capacity()/BATTERY_VOLTAGE_NOM)/1000.0);
  int solar_voltage = signpost_energy_get_solar_voltage();
  int solar_current = signpost_energy_get_solar_current();
  printf("/**************************************/\n");
  printf("\tBattery Voltage (mV): %d\tcurrent (uA): %d\n",battery_voltage,battery_current);
  printf("\tBattery remaining (mAh): %d\n",battery_energy);
  printf("\tSolar Voltage (mV): %d\tcurrent (uA): %d\n",solar_voltage,solar_current);
  printf("/**************************************/\n");

  batsol_buf[2] = ((battery_voltage & 0xFF00) >> 8);
  batsol_buf[3] = ((battery_voltage & 0xFF));
  batsol_buf[4] = ((battery_current & 0xFF000000) >> 24);
  batsol_buf[5] = ((battery_current & 0xFF0000) >> 16);
  batsol_buf[6] = ((battery_current & 0xFF00) >> 8);
  batsol_buf[7] = ((battery_current & 0xFF));
  batsol_buf[8] = ((solar_voltage & 0xFF00) >> 8);
  batsol_buf[9] = ((solar_voltage & 0xFF));
  batsol_buf[10] = ((solar_current & 0xFF000000) >> 24);
  batsol_buf[11] = ((solar_current & 0xFF0000) >> 16);
  batsol_buf[12] = ((solar_current & 0xFF00) >> 8);
  batsol_buf[13] = ((solar_current & 0xFF));
  batsol_buf[14] = battery_percent;
  batsol_buf[15] = ((battery_energy & 0xFF00) >> 8);
  batsol_buf[16] = ((battery_energy & 0xFF));
  batsol_buf[17] = ((battery_full & 0xFF00) >> 8);
  batsol_buf[18] = ((battery_full & 0xFF));

  int rc;
  if(!currently_initializing && num_inited > 1) {
    rc = signpost_networking_send_bytes(ModuleAddressRadio,batsol_buf,19);
    batsol_buf[1]++;
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
                    num_inited++;
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
    uint8_t message_type, size_t message_length, uint8_t* message) {

  printf("CALLBACK_ENERGY: received energy api callback of type %d\n",message_type);

  if (api_type != EnergyApiType) {
    signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
    return;
  }

  int rc;

  if (frame_type == NotificationFrame) {
      if(message_type == EnergyDutyCycleMessage) {
        printf("CALLBACK_ENERGY: received duty cycle request\n");
        //this is a duty cycle message
        //
        //how long does the module want to be duty cycled?
        uint32_t time;
        memcpy(&time, message, 4);

        //what module slot wants to be duty cycled?
        uint8_t mod = signpost_api_addr_to_mod_num(source_address);

        int timer = (int)(time/1000.0);
        printf("CALLBACK_ENERGY: Requested duty cycle for %ds\n",timer);
        //set up the value for the timer to check
        module_duty_cycle[mod] = (int)(time/1000.0);

        //turn it off
        printf("CALLBACK_ENERGY: Turning off module %d\n", mod);
        controller_module_disable_power(mod);
        controller_module_disable_i2c(mod);
      }
  } else if (frame_type == CommandFrame) {
    if (message_type == EnergyQueryMessage) {
      signpost_energy_information_t info;
      int limit = (int)(signpost_energy_get_module_energy_remaining(
                                    signpost_api_addr_to_mod_num(source_address))/1000.0);
      printf("CALLBACK_ENERGY: Calculated energy limit to be %d\n",limit);
      info.energy_limit_mWh = limit;
      int average = (int)(signpost_energy_get_module_average_power(
                                    signpost_api_addr_to_mod_num(source_address))/1000.0);
      printf("CALLBACK_ENERGY: Calculated average energy to be %d\n",average);
      info.average_power_mW = average;
      info.energy_limit_warning_threshold = ((info.energy_limit_mWh/info.average_power_mW) < 24);
      info.energy_limit_critical_threshold = ((info.energy_limit_mWh/info.average_power_mW) < 6);

      rc = signpost_energy_query_reply(source_address, &info);
      if (rc < 0) {
        printf(" - %d: Error sending energy query reply (code: %d). Replying with fail.\n", __LINE__, rc);
        signpost_api_error_reply_repeating(source_address, api_type, message_type, true, true, 1);
      }
    } else if (message_type == EnergyReportMessage) {
        //this is for the radio to report other module's energy usage
        printf("CALLBACK_ENERGY: Received energy report from 0x%.2x\n", source_address);

        //first we should get the message and unpack it
        signpost_energy_report_t report;
        memcpy(&report, message, message_length);

        //now we should convert the report module addresses to module slot numbers
        for(uint8_t i = 0; i < report.num_reports; i++) {
            printf("CALLBACK_ENERGY: Reported module address 0x%.2x in module slot %d for %d%% of energy\n",
                    report.reports[i].module_address,
                    signpost_api_addr_to_mod_num(report.reports[i].module_address),
                    report.reports[i].module_percent);
            report.reports[i].module_address = signpost_api_addr_to_mod_num(report.reports[i].module_address);
        }

        //now send the report to the energy
        signpost_energy_update_energy_from_report(signpost_api_addr_to_mod_num(source_address), &report);

        //reply to the report
        signpost_energy_report_reply(source_address, &report);
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

  static uint32_t count = 0;
  /*printf("\n\nGPS Data: %d:%02d:%02d.%lu %d/%d/%d\n",
          gps_data->hours, gps_data->minutes, gps_data->seconds, gps_data->microseconds,
          gps_data->month, gps_data->day, gps_data->year
          );

  printf("  Latitude:   %lu degrees\n", gps_data->latitude);
  printf("  Longitude:  %lu degrees\n", gps_data->longitude);*/

  //const char* fix_str = "Invalid fix";
  //if (gps_data->fix == 2) {
  //    fix_str = "2D fix";
  //} else if (gps_data->fix == 3) {
  //    fix_str = "3D fix";
  //}
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
  if(!currently_initializing && (count % 30) == 0 && count != 0 && num_inited > 1) {
    gps_buf[2] = _current_day;
    gps_buf[3] = _current_month;
    gps_buf[4] = _current_year;
    gps_buf[5] = _current_hour;
    gps_buf[6] = _current_minute;
    gps_buf[7] = _current_second;
    gps_buf[8] = ((_current_latitude & 0xFF000000) >> 24);
    gps_buf[9] = ((_current_latitude & 0x00FF0000) >> 16);
    gps_buf[10] = ((_current_latitude & 0x0000FF00) >> 8);
    gps_buf[11] = ((_current_latitude & 0x000000FF));
    gps_buf[12] = ((_current_longitude & 0xFF000000) >> 24);
    gps_buf[13] = ((_current_longitude & 0x00FF0000) >> 16);
    gps_buf[14] = ((_current_longitude & 0x0000FF00) >> 8);
    gps_buf[15] = ((_current_longitude & 0x000000FF));
    if(_current_satellite_count >= 3) {
      gps_buf[16] = 0x02;
    } else if (_current_satellite_count >=4) {
      gps_buf[16] = 0x03;
    } else {
      gps_buf[16] = 0x01;
    }
    gps_buf[17] = _current_satellite_count;
    rc = signpost_networking_send_bytes(ModuleAddressRadio,gps_buf,18);
    gps_buf[1]++;
  } else {
      rc = 0;
  }

  // Tickle the watchdog because something good happened.
  //app_watchdog_tickle_kernel();
  if (rc >= 0) {

  }

  watchdog_tickler(1);

  count++;
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
  energy_buf[0] = 0x01;
  energy_buf[1] = 0x00;

  energy_av_buf[0] = 0x04;
  energy_av_buf[1] = 0x00;

  gps_buf[0] = 0x02;
  gps_buf[1] = 0x00;

  batsol_buf[0] = 0x03;
  batsol_buf[1] = 0x00;

  // Configure FRAM
  // --------------
  printf("Configuring FRAM\n");
  fm25cl_set_read_buffer((uint8_t*) &fram, sizeof(controller_fram_t));
  fm25cl_set_write_buffer((uint8_t*) &fram, sizeof(controller_fram_t));

  // Read FRAM to see if anything is stored there
  const unsigned FRAM_MAGIC_VALUE = 0x49A80006;
  fm25cl_read_sync(0, sizeof(controller_fram_t));
  if (fram.magic == FRAM_MAGIC_VALUE) {
    // Great. We have saved data.
    // Initialize the energy algorithm with those values
    signpost_energy_init_ltc2943(&fram.energy);

    int bat = signpost_energy_get_battery_energy_remaining();

    printf("Energy remaining values saved in fram\n");
    printf("/**************************************/\n");
    printf("\tModule 0 energy remaining %d uWh\n",fram.energy.module_energy_remaining[0]);
    printf("\tModule 1 energy remaining %d uWh\n",fram.energy.module_energy_remaining[1]);
    printf("\tModule 2 energy remaining %d uWh\n",fram.energy.module_energy_remaining[2]);
    printf("\tModule 5 energy remaining %d uWh\n",fram.energy.module_energy_remaining[5]);
    printf("\tModule 6 energy remaining %d uWh\n",fram.energy.module_energy_remaining[6]);
    printf("\tModule 7 energy remaining %d uWh\n",fram.energy.module_energy_remaining[7]);
    printf("\tController energy remaining %d uWh\n",fram.energy.controller_energy_remaining);
    printf("\t Battery energy remaining %d uWh\n",bat);
    printf("/**************************************/\n");
  } else {
    // Initialize this
    fram.magic = FRAM_MAGIC_VALUE;

    //let the energy algorithm figure out how to initialize all the energies
    signpost_energy_init_ltc2943(NULL);

    fram.energy.controller_energy_remaining = signpost_energy_get_controller_energy_remaining();
    for(uint8_t i = 0; i < 8; i++) {
        fram.energy.module_energy_remaining[i] = signpost_energy_get_module_energy_remaining(i);
    }
    fm25cl_write_sync(0, sizeof(controller_fram_t));

    int bat = signpost_energy_get_battery_energy_remaining();

    printf("Energy remaining values initialized from battery capacity\n");
    printf("/**************************************/\n");
    printf("\tModule 0 energy remaining %d uWh\n",fram.energy.module_energy_remaining[0]);
    printf("\tModule 1 energy remaining %d uWh\n",fram.energy.module_energy_remaining[1]);
    printf("\tModule 2 energy remaining %d uWh\n",fram.energy.module_energy_remaining[2]);
    printf("\tModule 5 energy remaining %d uWh\n",fram.energy.module_energy_remaining[5]);
    printf("\tModule 6 energy remaining %d uWh\n",fram.energy.module_energy_remaining[6]);
    printf("\tModule 7 energy remaining %d uWh\n",fram.energy.module_energy_remaining[7]);
    printf("\tController energy remaining %d uWh\n",fram.energy.controller_energy_remaining);
    printf("\t Battery energy remaining %d uWh\n",bat);
    printf("/**************************************/\n");

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


  check_module_energy_remaining();

  ////////////////////////////////////////////////
  // Setup watchdog
  app_watchdog_set_kernel_timeout(180000);
  app_watchdog_start();

  printf("Everything intialized\n");


  printf("Entering loop\n");

  delay_ms(3000);

  uint32_t index = 1;
  while(1) {
    // always check for modules that need to be initialized
    check_module_initialization();

    //check for any modules that need to be turned back on
    check_module_duty_cycle();

    // get energy updates every 10 seconds
    if ((index % 60) == 0) {
      printf("CONTROLLER_STATE: Check energy\n");
      get_energy_remaining();
    }

    if((index %60) == 20) {
      get_batsol();
    }

    if ((index % 60) == 40) {
      printf("CONTROLLER_STATE: Check energy\n");
      get_energy_average();
    }

    if ((index % 50) == 0 && index != 0) {
        if(mod_isolated_out < 0) {
            check_watchdogs();
        }
    }

    if ((index % 300) == 0) {
        printf("CONTROLLER_STATE: updating energy remaining\n");
        signpost_energy_update_energy();

        check_module_energy_remaining();
    }

    index++;
    delay_ms(1000);
  }
}

