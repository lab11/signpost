#pragma once

#include <stdint.h>

typedef struct {
    // date
    uint8_t  day;
    uint8_t  month;
    uint16_t year;

    // time
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint32_t microseconds;

    // location
    uint32_t latitude;
    uint32_t longitude;

    // quality
    uint8_t fix; // 1=No fix, 2=2D, 3=3D
    uint8_t satellite_count; // satellites used in fix
} gps_data_t;

// initialize the GPS
void gps_init ();

// start receiving GPS data
void gps_start (void (*callback)(gps_data_t*));

