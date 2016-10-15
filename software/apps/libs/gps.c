#include "gps.h"
#include "tock_str.h"
#include "minmea.h"

#include <string.h>

// local data
static char gps_data_buffer[500] = {0};
static gps_data_t gps_data = {0};
static void (*gps_callback)(gps_data_t*) = NULL;
static bool continuous_mode = false;

// useful GPS settings transmissions
char* GPS_NORMAL_OPERATION = "$PMTK225,0*";
char* GPS_ACTIVATE_GSA_RMC = "$PMTK314,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*";
char* GPS_ACTIVATE_ALL_MSGS = "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*";
char* GPS_DGPS_RTCM = "$PMTK301,1*";
char* GPS_DGPS_WAAS = "$PMTK301,2*";
char* GPS_SBAS_ENABLE = "$PMTK313,1*";
char* GPS_DATUM_US = "$PMTK330,142*";
char* GPS_DATUM_GLOBAL = "$PMTK330,0*";

// internal prototypes
void gps_send_msg (char* msg);
void gps_rx_callback (int len, int y, int z, void* userdata);
bool gps_parse_data (char* line);
int32_t to_decimal_degrees (struct minmea_float coor);


// global functions

void gps_init () {
    // non-duty cycle mode
    gps_send_msg(GPS_NORMAL_OPERATION);

    // only get GSA and RMC messages
    gps_send_msg(GPS_ACTIVATE_GSA_RMC);
    
    // enable differential GPS mode
    gps_send_msg(GPS_DGPS_WAAS);
    gps_send_msg(GPS_SBAS_ENABLE);

    // use the US datum
    gps_send_msg(GPS_DATUM_US);
}

void gps_continuous (void (*callback)(gps_data_t*)) {
    // store user callback
    gps_callback = callback;
    continuous_mode = true;

    // start listening for data
    getauto(gps_data_buffer, 500, gps_rx_callback, NULL);
}

void gps_sample (void (*callback)(gps_data_t*)) {
    // store user callback
    gps_callback = callback;
    continuous_mode = false;

    // start listening for data
    getauto(gps_data_buffer, 500, gps_rx_callback, NULL);
}


// local functions

void gps_send_msg (char* msg) {
    printf("%s%02X\r\n", msg, minmea_checksum(msg));
}

void gps_rx_callback (int len, int y, int z, void* userdata) {

    // parse data
    bool data_updated = false;
    char* line = NULL;
    line = strtok(gps_data_buffer, "\n");
    while (line != NULL) {
        if (gps_parse_data(line)) {
            data_updated = true;
        }
        line = strtok(NULL, "\n");
    }

    // listen for next GPS data
    if (continuous_mode == true || data_updated == false) {
        getauto(gps_data_buffer, 500, gps_rx_callback, NULL);
    }

    // call back user
    if (gps_callback != NULL && data_updated == true) {
        gps_callback(&gps_data);
    }
}

bool gps_parse_data (char* line) {
    bool data_updated = false;

    if (line != NULL) {
        struct minmea_sentence_gsa gsa_frame;
        struct minmea_sentence_rmc rmc_frame;
        switch (minmea_sentence_id(line, false)) {
            case MINMEA_SENTENCE_GSA:
                if (minmea_parse_gsa(&gsa_frame, line)) {
                    // get fix. 1=No fix, 2=2D, 3=3D
                    gps_data.fix = (uint8_t)gsa_frame.fix_type;

                    // count how many satellites were used in fix
                    uint8_t satellite_count = 0;
                    for (int i=0; i<12; i++) {
                        if (gsa_frame.sats[i] != 0) {
                            satellite_count++;
                        }
                    }
                    gps_data.satellite_count = satellite_count;

                    data_updated = true;
                }
                break;
            case MINMEA_SENTENCE_RMC:
                if (minmea_parse_rmc(&rmc_frame, line)) {
                    // get date fields
                    gps_data.day   = (uint8_t)rmc_frame.date.day;
                    gps_data.month = (uint8_t)rmc_frame.date.month;
                    gps_data.year  = (uint16_t)rmc_frame.date.year;

                    // get time fields
                    gps_data.hours        = (uint8_t)rmc_frame.time.hours;
                    gps_data.minutes      = (uint8_t)rmc_frame.time.minutes;
                    gps_data.seconds      = (uint8_t)rmc_frame.time.seconds;
                    gps_data.microseconds = (uint32_t)rmc_frame.time.microseconds;

                    // location
                    gps_data.latitude = to_decimal_degrees(rmc_frame.latitude);
                    gps_data.longitude = to_decimal_degrees(rmc_frame.longitude);

                    data_updated = true;
                }
                break;
            default:
                // do nothing. I don't care about this data
                break;
        }
    }

    return data_updated;
}

int32_t to_decimal_degrees (struct minmea_float coor) {
    int16_t degrees = coor.value/(coor.scale*100);
    int32_t minutes = coor.value - degrees*(coor.scale*100);
    int32_t decimal_degrees = degrees*(coor.scale*100) + minutes/60*100;

    return decimal_degrees;
}

