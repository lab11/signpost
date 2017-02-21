// Runs on the ambient module. Reads sensor data and HTTP Posts it over the
// Signpost API

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

// tock includes
#include "led.h"
#include "isl29035.h"
#include "si7021.h"
#include "timer.h"
#include "tock.h"

// signpost includes
#include "app_watchdog.h"
#include "signpost_api.h"
#include "simple_post.h"

// module-specific settings
#define AMBIENT_MODULE_I2C_ADDRESS 0x32
#define AMBIENT_LED1 2

// stored sensor readings
typedef struct {
  int light;
  int temperature;
  unsigned humidity;
  int err_code;
} Sensor_Data_t;
static Sensor_Data_t samples = {0};

// keep track of whether functions succeeded
static bool sample_sensors_successful = true;
static bool post_over_http_successful = true;

static void sample_sensors (void) {
  // read data from sensors and save locally
  int err_code = SUCCESS;
  sample_sensors_successful = true;

  // get light
  int light = 0;
  err_code = isl29035_read_light_intensity();
  if (err_code < SUCCESS) {
    printf("Error reading from light sensor: %d\n", light);
  } else {
    light = err_code;
    err_code = SUCCESS;
  }

  // get temperature and humidity
  int temperature = 0;
  unsigned humidity = 0;
  int err = si7021_get_temperature_humidity_sync(&temperature, &humidity);
  if (err < SUCCESS) {
    printf("Error reading from temperature/humidity sensor: %d\n", err);
    err_code = err;
  }

  // print readings
  printf("--Sensor readings--\n");
  printf("\tTemperature %d (degrees C * 100)\n", temperature);
  printf("\tHumidity %d (%%RH * 100)\n", humidity);
  printf("\tLight %d (lux)\n", light);

  // store readings
  samples.light = light;
  samples.temperature = temperature;
  samples.humidity = humidity;
  samples.err_code = err_code;

  // track success
  if (err_code != SUCCESS) {
    sample_sensors_successful = false;
  }
}

static void post_over_http (void) {
  // post sensor data over HTTP and get response
  int err_code = SUCCESS;
  post_over_http_successful = true;

  // URL for an HTTP POST testing service
  const char* url = "posttestserver.com/post.php";

  // http post data
  printf("--POSTing data--\n");
  int response = simple_octetstream_post(url, (uint8_t*)&samples, sizeof(Sensor_Data_t));
  if (response < SUCCESS) {
    printf("Error posting: %d\n", response);
  } else {
    printf("\tResponse: %d\n", response);
  }

  if (err_code != SUCCESS) {
    post_over_http_successful = false;
  }
}

static void tickle_watchdog (void) {
  // keep the watchdog from resetting us if everything is successful

  if (sample_sensors_successful && post_over_http_successful) {
    app_watchdog_tickle_kernel();
    led_toggle(AMBIENT_LED1);
  }
}


int main (void) {
  printf("\n[Ambient Module] Sample and Post\n");

  // initialize module as a part of the signpost bus
  signpost_initialization_module_init(AMBIENT_MODULE_I2C_ADDRESS, NULL);
  printf(" * Bus initialized\n");

  // set up watchdog
  // Resets after 10 seconds without a valid response
  app_watchdog_set_kernel_timeout(10000);
  app_watchdog_start();
  printf(" * Watchdog started\n");

  printf(" * Initialization complete\n\n");

  // perform main application
  while (true) {
    // sample from onboard sensors
    sample_sensors();

    // send HTTP POST over Signpost API
    post_over_http();

    // check the watchdog
    tickle_watchdog();
    printf("\n");

    // sleep for a bit
    delay_ms(3000);
  }
}

