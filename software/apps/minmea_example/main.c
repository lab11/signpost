#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <tock_str.h>
#include <gpio.h>

#include "minmea.h"

char str_buffer[500] = {0};
bool data_ready = false;

void getline_cb(int len, int y, int z, void* userdata) {
    data_ready = true;
    str_buffer[len] = 0;

    getauto(str_buffer, 500, getline_cb, NULL);
}

int32_t to_decimal_degrees (struct minmea_float coor) {
    int16_t degrees = coor.value/(coor.scale*100);
    int32_t minutes = coor.value - degrees*(coor.scale*100);
    int32_t decimal_degrees = degrees*(coor.scale*100) + minutes/60*100;

    return decimal_degrees;
}

char print_buf[500] = {0};
void parse_data (char* line) {
    sprintf(print_buf, "%d   ", strlen(line));

    char* print_str = &print_buf[3];
    if (line != NULL) {
        struct minmea_sentence_rmc rmc_frame;
        struct minmea_sentence_gga gga_frame;
        struct minmea_sentence_gsa gsa_frame;
        struct minmea_sentence_gll gll_frame;
        struct minmea_sentence_gst gst_frame;
        struct minmea_sentence_gsv gsv_frame;
        struct minmea_sentence_vtg vtg_frame;
        switch (minmea_sentence_id(line, false)) {
            case MINMEA_SENTENCE_RMC:
                if (minmea_parse_rmc(&rmc_frame, line)) {
                    sprintf(print_str, "RMC: lat %d:%d [%d] lon %d:%d [%d] M/D/Y %d/%d/%d\n",
                            rmc_frame.latitude.value, rmc_frame.latitude.scale, to_decimal_degrees(rmc_frame.latitude),
                            rmc_frame.longitude.value, rmc_frame.longitude.scale, to_decimal_degrees(rmc_frame.longitude),
                            rmc_frame.date.month, rmc_frame.date.day, rmc_frame.date.year);
                }
                break;
            case MINMEA_SENTENCE_GGA:
                if (minmea_parse_gga(&gga_frame, line)) {
                    sprintf(print_str, "GGA: time %d:%d:%d.%d fix %d sats %d lat %d:%d lon %d:%d alt %d:%d\n",
                            gga_frame.time.hours, gga_frame.time.minutes,
                            gga_frame.time.seconds, gga_frame.time.microseconds,
                            gga_frame.fix_quality, gga_frame.satellites_tracked,
                            gga_frame.latitude.value, gga_frame.latitude.scale,
                            gga_frame.longitude.value, gga_frame.longitude.scale,
                            gga_frame.altitude.value, gga_frame.altitude.scale);
                }
                break;
            case MINMEA_SENTENCE_GSA:
                if (minmea_parse_gsa(&gsa_frame, line)) {
                    char* mode_str;
                    if (gsa_frame.mode == 'A') {
                        mode_str = "Auto";
                    } else if (gsa_frame.mode == 'M') {
                        mode_str = "Force";
                    } else {
                        mode_str = "Invalid Mode";
                    }

                    char* fix_str;
                    if (gsa_frame.fix_type == 1) {
                        fix_str = "No Fix";
                    } else if (gsa_frame.fix_type == 2) {
                        fix_str = "2D Fix";
                    } else if (gsa_frame.fix_type == 3) {
                        fix_str = "3D Fix";
                    } else {
                        fix_str = "Invalid Fix";
                    }

                    sprintf(print_str, "GSA: %s %s sat# %d %d %d %d %d %d %d %d %d %d %d %d\n",
                            mode_str, fix_str,
                            gsa_frame.sats[0], gsa_frame.sats[1], gsa_frame.sats[2],
                            gsa_frame.sats[3], gsa_frame.sats[4], gsa_frame.sats[5],
                            gsa_frame.sats[6], gsa_frame.sats[7], gsa_frame.sats[8],
                            gsa_frame.sats[9], gsa_frame.sats[10], gsa_frame.sats[11]
                            );
                }
                break;
            case MINMEA_SENTENCE_GLL:
                if (minmea_parse_gll(&gll_frame, line)) {
                    sprintf(print_str, "GLL: lat %d:%d lon %d:%d time %d:%d:%d.%d Status %c Mode %c\n",
                            gll_frame.latitude.value, gll_frame.latitude.scale,
                            gll_frame.longitude.value, gll_frame.longitude.scale,
                            gll_frame.time.hours, gll_frame.time.minutes,
                            gll_frame.time.seconds, gll_frame.time.microseconds,
                            gll_frame.status, gll_frame.mode
                            );
                }
                break;
            case MINMEA_SENTENCE_GST:
                sprintf(print_str, "GST: unsupported on RXM\n");
                break;
            case MINMEA_SENTENCE_GSV:
                if (minmea_parse_gsv(&gsv_frame, line)) {
                    sprintf(print_str, "GSV: %d of %d sats %d sat#(snr) %d(%d) %d(%d) %d(%d) %d(%d)\n",
                            gsv_frame.msg_nr, gsv_frame.total_msgs, gsv_frame.total_sats,
                            gsv_frame.sats[0].nr, gsv_frame.sats[0].snr,
                            gsv_frame.sats[1].nr, gsv_frame.sats[1].snr,
                            gsv_frame.sats[2].nr, gsv_frame.sats[2].snr,
                            gsv_frame.sats[3].nr, gsv_frame.sats[3].snr
                            );
                }
                break;
            case MINMEA_SENTENCE_VTG:
                if (minmea_parse_vtg(&vtg_frame, line)) {
                    sprintf(print_str, "VTG: speed %d:%d Mode %c\n",
                            vtg_frame.speed_kph.value, vtg_frame.speed_kph.scale,
                            vtg_frame.faa_mode);
                }
                break;
            case MINMEA_UNKNOWN:
                sprintf(print_str, "Unknown line %s\n", line);
                break;
            case MINMEA_INVALID:
                {
                char type[6];
                    if (!minmea_check(line, false)) {
                        sprintf(print_str, "Invalid (fails check) %s\n", line);
                    } else
                        if (!minmea_scan(line, "t", type)) {
                        sprintf(print_str, "Invalid (fails scan) %s\n", line);
                    } else {
                        sprintf(print_str, "Invalid (other?) %s\n", line);
                    }
                    break;
                }
        }
    } else {
        sprintf(print_str, "Null line %s\n", str_buffer);
    }

    print_buf[499] = 0;
    putnstr(print_buf, strlen(print_buf));
}

void gps_send_msg (char* msg) {
    printf("%s%02X\r\n", msg, minmea_checksum(msg));
}

char* GPS_NORMAL_OPERATION = "$PMTK225,0*";
char* GPS_ACTIVATE_GSA_RMC = "$PMTK314,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0*";
char* GPS_ACTIVATE_ALL_MSGS = "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*";
char* GPS_DGPS_RTCM = "$PMTK301,1*";
char* GPS_DGPS_WAAS = "$PMTK301,2*";
char* GPS_SBAS_ENABLE = "$PMTK313,1*";
char* GPS_DATUM_US = "$PMTK330,142*";
char* GPS_DATUM_GLOBAL = "$PMTK330,0*";


void main() {
    printf("GPS Test\n");
    delay_ms(500);

    // set up GPS
    gps_send_msg(GPS_NORMAL_OPERATION);
    gps_send_msg(GPS_ACTIVATE_ALL_MSGS);
    gps_send_msg(GPS_DGPS_WAAS);
    gps_send_msg(GPS_SBAS_ENABLE);
    gps_send_msg(GPS_DATUM_US);

    getauto(str_buffer, 500, getline_cb, NULL);

    while (1) {
        
        if (data_ready) {
            data_ready = false;

            char* line = NULL;
            line = strtok(str_buffer, "\n");
            while ( line != NULL) {
                //char* line = strchr(str_buffer, '$');
                parse_data(line);
                line = strtok(NULL, "\n");
            }
            putstr("---\n");
        }

        yield();
    }
}

