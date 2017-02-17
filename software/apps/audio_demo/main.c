
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

#define STROBE 5
#define RESET 6
#define POWER 7
#define GREEN_LED 0
#define RED_LED 1
#define BUFFER_SIZE 20


uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];

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

    gpio_enable_output(8);
    gpio_enable_output(9);
    gpio_clear(8);
    gpio_clear(9);

    //init i2c
    //i2c_master_slave_set_callback(i2c_master_slave_callback, NULL);
    i2c_master_slave_set_slave_address(0x33);

    //i2c_master_slave_set_master_read_buffer(master_read_buf, BUFFER_SIZE);
    i2c_master_slave_set_master_write_buffer(master_write_buf, BUFFER_SIZE);
    //i2c_master_slave_set_slave_read_buffer(slave_read_buf, BUFFER_SIZE);
    //i2c_master_slave_set_slave_write_buffer(slave_write_buf, BUFFER_SIZE);

    //i2c_master_slave_listen();

    master_write_buf[0] = 0x33;
    master_write_buf[1] = 0x01;

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
    //i2c_master_slave_write(0x22,16);

    delay_ms(1000);
    gpio_set(8);
    gpio_set(9);

    // start up the app watchdog
    app_watchdog_set_kernel_timeout(10000);
    app_watchdog_start();

    uint8_t count = 50;
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
            master_write_buf[2+i*2] = (uint8_t)((data >> 8) & 0xff);
            master_write_buf[2+i*2+1] = (uint8_t)(data & 0xff);
            delay_ms(1);
            gpio_set(STROBE);
            delay_ms(1);
            gpio_clear(STROBE);
        }
        uint16_t data = (uint16_t)adc_read_single_sample(0);
        master_write_buf[14] = (uint8_t)((data >> 8) & 0xff);
        master_write_buf[15] = (uint8_t)(data & 0xff);

        //give some indication of volume to the user
        if((uint16_t)((master_write_buf[6] << 8) + master_write_buf[7]) > 400) {
            //turn on green LED
            led_on(GREEN_LED);
        } else {
            led_off(GREEN_LED);
        }
        if((uint16_t)((master_write_buf[6] << 8) + master_write_buf[7]) > 3500) {
            //turn on red LED
            led_on(RED_LED);
        } else {
            led_off(RED_LED);
        }

        count++;
        if(count >= 50) {
            i2c_master_slave_write_sync(0x22,16);
            //i2c_master_slave_write(0x22,16);
            count = 0;
        }

        // well we made it through a loop. Seems as good a time to tickle as any
        app_watchdog_tickle_kernel();
    }
}
