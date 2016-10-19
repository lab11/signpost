#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <console.h>
#include <gpio.h>

#include "gps.h"
#include "minmea.h"

void gps_callback (gps_data_t* gps_data) {
    // got new gps data

    printf("GPS Data: %d:%d:%d.%d %d/%d/%d\n",
            gps_data->hours, gps_data->minutes, gps_data->seconds, gps_data->microseconds,
            gps_data->month, gps_data->day, gps_data->year
            );

    printf("\t%d degrees lat - %d degrees lon\n",
            gps_data->latitude, gps_data->longitude);

    char* fix_str = "Invalid fix";
    if (gps_data->fix == 2) {
        fix_str = "2D fix";
    } else if (gps_data->fix == 3) {
        fix_str = "3D fix";
    }
    printf("\tfix %s sats %d\n",
            fix_str, gps_data-> satellite_count);
}

static void timer_callback (
        int callback_type __attribute__ ((unused)),
        int pin_value __attribute__ ((unused)),
        int unused __attribute__ ((unused)),
        void* callback_args __attribute__ ((unused))
        ) {

    // get a single gps update
    gps_sample(gps_callback);
}


void main() {
    printf("GPS Test\n");
    delay_ms(500);

    // initialize and begin collecting gps data
    gps_init();

    // option 1:
    // use GPS with a timer
    timer_subscribe(timer_callback, NULL);
    timer_start_repeating(7000);

    // other option:
    // get gps updates continuously
    //gps_continuous(gps_callback);

    while (1) {
        yield();
    }
}

