#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_watchdog.h"
#include "bonus_timer.h"
#include "console.h"
#include "controller.h"
#include "fm25cl.h"
#include "gpio_async.h"
#include "gps.h"
#include "i2c_master_slave.h"
#include "minmea.h"
#include "signpost_energy.h"
#include "timer.h"
#include "tock.h"

static void get_energy (void);
static void gps_callback (gps_data_t* gps_data);


uint8_t slave_read_buf[256];
uint8_t slave_write_buf[256];
uint8_t master_read_buf[256];
uint8_t master_write_buf[256];

uint8_t fm25cl_read_buf[256];
uint8_t fm25cl_write_buf[256];





static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
  get_energy();
}

static void bonus_timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {
  gps_sample(gps_callback);
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
    sprintf(buf, "  Controller energy: %i uAh\n", energy);
  } else if (module == 4) {
    sprintf(buf, "  Linux energy:      %i uAh\n", energy);
  } else {
    sprintf(buf, "  Module %i energy:   %i uAh\n", module, energy);
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

  // My address
  master_write_buf[0] = 0x20;
  // Packet type. 1 == Energy Meters
  master_write_buf[1] = 0x01;

  master_write_buf[2]  = (uint8_t) (((fram.energy_module0 / 1000) >> 8) & 0xFF);
  master_write_buf[3]  = (uint8_t) ((fram.energy_module0 / 1000) & 0xFF);
  master_write_buf[4]  = (uint8_t) (((fram.energy_module1 / 1000) >> 8) & 0xFF);
  master_write_buf[5]  = (uint8_t) ((fram.energy_module1 / 1000) & 0xFF);
  master_write_buf[6]  = (uint8_t) (((fram.energy_module2 / 1000) >> 8) & 0xFF);
  master_write_buf[7]  = (uint8_t) ((fram.energy_module2 / 1000) & 0xFF);
  master_write_buf[8]  = (uint8_t) (((fram.energy_controller / 1000) >> 8) & 0xFF);
  master_write_buf[9]  = (uint8_t) ((fram.energy_controller / 1000) & 0xFF);
  master_write_buf[10] = (uint8_t) (((fram.energy_linux / 1000) >> 8) & 0xFF);
  master_write_buf[11] = (uint8_t) ((fram.energy_linux / 1000) & 0xFF);
  master_write_buf[12] = (uint8_t) (((fram.energy_module5 / 1000) >> 8) & 0xFF);
  master_write_buf[13] = (uint8_t) ((fram.energy_module5 / 1000) & 0xFF);
  master_write_buf[14] = (uint8_t) (((fram.energy_module6 / 1000) >> 8) & 0xFF);
  master_write_buf[15] = (uint8_t) ((fram.energy_module6 / 1000) & 0xFF);
  master_write_buf[16] = (uint8_t) (((fram.energy_module7 / 1000) >> 8) & 0xFF);
  master_write_buf[17] = (uint8_t) ((fram.energy_module7 / 1000) & 0xFF);

  int result = i2c_master_slave_write_sync(0x22, 18);
  {
    char buf[64];
    sprintf(buf, "Return from I2C Write: %i\n", result);
    putstr(buf);
  }

  // Only say things are working if i2c worked
  if (result == 0) {
    // Tickle the watchdog because something good happened.
    //app_watchdog_tickle_kernel();
    watchdog_tickler(2);
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

  // My address
  master_write_buf[0] = 0x20;
  // Packet type. 2 == GPS
  master_write_buf[1] = 0x02;
  // date
  master_write_buf[2] = (uint8_t) (gps_data->day   & 0xFF);
  master_write_buf[3] = (uint8_t) (gps_data->month & 0xFF);
  master_write_buf[4] = (uint8_t) (gps_data->year  & 0xFF);
  // time
  master_write_buf[5] = (uint8_t) (gps_data->hours   & 0xFF);
  master_write_buf[6] = (uint8_t) (gps_data->minutes & 0xFF);
  master_write_buf[7] = (uint8_t) (gps_data->seconds & 0xFF);
  // latitude
  master_write_buf[8]  = (uint8_t) ((gps_data->latitude >> 24) & 0xFF);
  master_write_buf[9]  = (uint8_t) ((gps_data->latitude >> 16) & 0xFF);
  master_write_buf[10] = (uint8_t) ((gps_data->latitude >>  8) & 0xFF);
  master_write_buf[11] = (uint8_t) ((gps_data->latitude)       & 0xFF);
  // longitude
  master_write_buf[12] = (uint8_t) ((gps_data->longitude >> 24) & 0xFF);
  master_write_buf[13] = (uint8_t) ((gps_data->longitude >> 16) & 0xFF);
  master_write_buf[14] = (uint8_t) ((gps_data->longitude >>  8) & 0xFF);
  master_write_buf[15] = (uint8_t) ((gps_data->longitude)       & 0xFF);
  // quality
  master_write_buf[16] = (uint8_t) (gps_data->fix & 0xFF);
  master_write_buf[17] = (uint8_t) (gps_data->satellite_count & 0xFF);

  int result = i2c_master_slave_write_sync(0x22, 18);

  // Only say things are working if i2c worked
  if (result == 0) {
    // Tickle the watchdog because something good happened.
    //app_watchdog_tickle_kernel();
    watchdog_tickler(1);
  }
}

int main (void) {
  putstr("[Controller] ** Main App **\n");

  // Setup backplane by enabling the modules
  controller_init_module_switches();
  controller_all_modules_enable_power();
  controller_all_modules_enable_i2c();
  controller_all_modules_disable_usb();
  // controller_all_modules_disable_i2c();
  // controller_module_enable_i2c(MODULE5);
  // controller_module_enable_i2c(MODULE0);

  i2c_master_slave_set_slave_address(0x20);

  // i2c_master_slave_set_master_read_buffer(master_read_buf, 256);
  i2c_master_slave_set_master_write_buffer(master_write_buf, 256);
  // i2c_master_slave_set_slave_read_buffer(slave_read_buf, 256);
  // i2c_master_slave_set_slave_write_buffer(slave_write_buf, 256);

  putstr("Configuring FRAM\n");
  // Configure FRAM
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

  putstr("Init'ing energy\n");
  // Need to init the signpost energy library
  signpost_energy_init();

  putstr("Resetting energy\n");
  // Reset all of the LTC2941s
  signpost_energy_reset();

  putstr("GPS\n");
  // setup gps
  gps_init();

  putstr("Starting timers\n");
  // Need a timer
  timer_subscribe(timer_callback, NULL);
  bonus_timer_subscribe(bonus_timer_callback, NULL);

  // Start timers to read energy and GPS
  timer_start_repeating(10000);
  bonus_timer_start_repeating(27000);

  // Setup watchdog
  //app_watchdog_set_kernel_timeout(30000);
  //app_watchdog_start();

  putstr("Everything intialized\n");
}
