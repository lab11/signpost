
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
//#include <math.h>

#include <tock.h>
#include <console.h>
#include "firestorm.h"
#include "gpio.h"
#include "led.h"
#include "adc.h"
#include "app_watchdog.h"
#include "msgeq7.h"
#include "i2c_master_slave.h"
#include "timer.h"
#include "bonus_timer.h"
#include "signpost_api.h"

#define STROBE 3
#define RESET 4
#define POWER 5
#define GREEN_LED 2
#define RED_LED 3
#define BUFFER_SIZE 20

uint8_t send_buf[20];
bool sample_done = false;
bool still_sampling = false;
int send_callback_ms = 10000;

//gain = 20k resistance
#define PREAMP_GAIN 22.5
#define MSGEQ7_GAIN 22.0
#define SPL 94.0
//the magic number is a combination of:
//voltage ref
//bits of adc precision
//vpp to rms conversion
//microphone sensitivity
#define MAGIC_NUMBER 43.75

/*static uint16_t convert_to_dB(uint16_t output) {
    return (uint16_t)(((20*log10(output/MAGIC_NUMBER)) + SPL - PREAMP_GAIN - MSGEQ7_GAIN)*10);
}*/

static void delay() {
    for(volatile uint16_t i = 0; i < 2000; i++);
}

int bands_total[7] = {0};
int bands_max[7] = {0};
int bands_now[7] = {0};
int bands_num[7] =  {0};

static void adc_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int sample,
        void* callback_args __attribute__ ((unused))
        ) {

    static uint8_t i = 0;

    //send_buf[2+i*2] = (uint8_t)((sample >> 8) & 0xff);
    //send_buf[2+i*2+1] = (uint8_t)(sample & 0xff);
    bands_total[i] += sample;
    if(sample > bands_max[i]) {
        bands_max[i] = sample;
    }
    bands_now[i] = sample;
    bands_num[i]++;
    delay();
    gpio_set(STROBE);
    delay();
    gpio_clear(STROBE);

    if(i == 6) {

        if(bands_now[3] > 400) {
            //turn on green LED
            led_on(GREEN_LED);
        } else {
            led_off(GREEN_LED);
        }
        if(bands_now[3] > 3500) {
            //turn on red LED
            led_on(RED_LED);
        } else {
            led_off(RED_LED);
        }

        delay();
        gpio_set(STROBE);
        gpio_set(RESET);
        delay();
        gpio_clear(STROBE);
        delay();
        gpio_clear(RESET);
        gpio_set(STROBE);
        delay();
        gpio_clear(STROBE);

        i = 0;
        still_sampling = true;

    } else {

        i++;

    }

    sample_done = true;
}

static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {

    static int index = 1;
    int rc;
    //printf("About to send data to radio\n");

    for(uint8_t j = 0; j < 7; j++) {
        send_buf[2+j*2] = (uint8_t)((bands_max[j] >> 8) & 0xff);
        send_buf[2+j*2+1] = (uint8_t)(bands_max[j] & 0xff);
    }

    rc = signpost_networking_send_bytes(ModuleAddressRadio,send_buf,16);
    rc = 1;
    send_buf[1]++;
    //printf("Sent data with return code %d\n\n\n",rc);


    //reset all the variables for the next period
    for(uint8_t j = 0; j < 7; j++) {
        bands_total[j] = 0;
        bands_now[j] = 0;
        bands_max[j] = 0;
        bands_num[j] = 0;
    }

    if(rc >= 0 && still_sampling == true) {
        app_watchdog_tickle_kernel();
        still_sampling = false;
    }

    //this is how we will do energy adaptivity
    //every 10 minutes do an energy query
    //If we are using too much energy then back off
    if(index > (1200.0/(send_callback_ms/1000.0))) {
        signpost_energy_information_t e;
        rc = signpost_energy_query(&e);
        printf("Received return code %d\n",rc);
        if(rc < 0) {
            return;
        }

        printf("Energy limit %d\n", e.energy_limit_mWh);
        printf("Energy average %d\n", e.average_power_mW);
        if(e.energy_limit_mWh/e.average_power_mW < 48) {
            send_callback_ms += 1000;
        } else {
            send_callback_ms -= 1000;
        }

        if(send_callback_ms < 1000) {
            send_callback_ms = 1000;
        }

        printf("setting timer to %d\n",send_callback_ms);
        timer_start_repeating(send_callback_ms);
        index = 0;
    } else {
        index++;
    }
}


int main (void) {

    //initialize the signpost API
    int rc;
    do {
        rc = signpost_initialization_module_init(0x33, NULL);
        if (rc < 0) {
            printf(" - Error initializing bus (code %d). Sleeping for 5s\n",rc);
            delay_ms(5000);
        }
    } while (rc < 0);
    printf(" * Bus initialized\n");

    /*do {
        rc = signpost_watchdog_start();
        if(rc < 0) {
            delay_ms(1000);
        }
    } while (rc < 0);*/

    //printf("Watchdog Started");


    gpio_enable_output(8);
    gpio_enable_output(9);
    gpio_clear(8);
    gpio_clear(9);

    send_buf[0] = 0x01;
    send_buf[1] = 0x00;


    gpio_enable_output(STROBE);
    gpio_enable_output(RESET);
    gpio_enable_output(POWER);

    gpio_clear(POWER);
    gpio_clear(STROBE);
    gpio_clear(RESET);


    // start up the app watchdog
    app_watchdog_set_kernel_timeout(60000);
    app_watchdog_start();

    //init adc
    adc_set_callback(adc_callback, NULL);
    adc_initialize();

    //start timer
    timer_subscribe(timer_callback, NULL);
    timer_start_repeating(send_callback_ms);

    while (1) {
        sample_done = false;
        adc_single_sample(0);
        yield_for(&sample_done);
    }
}
