
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
#include "signpost_api.h"

#define STROBE 3
#define RESET 4
#define POWER 5
#define GREEN_LED 2
#define RED_LED 3
#define BUFFER_SIZE 20

uint8_t send_buf[20];

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


/*
static void i2c_master_slave_callback(int callback_type, int length, int unused, void* callback_args) {
    return;
}
*/

/*static void  timer_callback( int callback_type , int channel, int data, void* callback_args) {
  i2c_master_slave_write(0x22,16);
  }*/

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

    gpio_enable_output(8);
    gpio_enable_output(9);
    gpio_clear(8);
    gpio_clear(9);

    send_buf[0] = 0x01;

    //init adc
    //adc_set_callback(adc_callback, (void*)&result);
    adc_initialize();
    gpio_enable_output(STROBE);
    gpio_enable_output(RESET);
    gpio_enable_output(POWER);

    gpio_clear(POWER);
    gpio_clear(STROBE);
    gpio_clear(RESET);

    //timer_subscribe(timer_callback, NULL);
    //timer_start_repeating(1000);
    delay_ms(1000);
    gpio_set(8);
    gpio_set(9);

    // start up the app watchdog
    app_watchdog_set_kernel_timeout(60000);
    app_watchdog_start();

    uint16_t count = 50;
    while (1) {
        delay_ms(1);
        gpio_set(STROBE);
        gpio_set(RESET);
        delay_ms(1);
        gpio_clear(STROBE);
        delay_ms(1);
        gpio_clear(RESET);
        gpio_set(STROBE);
        delay_ms(1);
        gpio_clear(STROBE);

        for(uint8_t i = 0; i < 6; i++) {
            uint16_t data = (uint16_t)adc_read_single_sample(0);
            send_buf[1+i*2] = (uint8_t)((data >> 8) & 0xff);
            send_buf[1+i*2+1] = (uint8_t)(data & 0xff);
            delay_ms(1);
            gpio_set(STROBE);
            delay_ms(1);
            gpio_clear(STROBE);
        }
        uint16_t data = (uint16_t)adc_read_single_sample(0);
        send_buf[13] = (uint8_t)((data >> 8) & 0xff);
        send_buf[14] = (uint8_t)(data & 0xff);

        //give some indication of volume to the user
        if((uint16_t)((send_buf[5] << 8) + send_buf[6]) > 400) {
            //turn on green LED
            led_on(GREEN_LED);
        } else {
            led_off(GREEN_LED);
        }
        if((uint16_t)((send_buf[7] << 8) + send_buf[8]) > 3500) {
            //turn on red LED
            led_on(RED_LED);
        } else {
            led_off(RED_LED);
        }

        count++;
        if(count >= 500) {
            printf("About to send data\n");
            rc = signpost_networking_send_bytes(ModuleAddressRadio,send_buf,15);
            printf("Send data with return code %d\n",rc);
            if(rc >= 0) {
                app_watchdog_tickle_kernel();
            }
            count = 0;
        }
    }
}
