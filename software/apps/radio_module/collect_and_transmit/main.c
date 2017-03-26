#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

//nordic includes
#include "nrf.h"
#include <nordic_common.h>
#include <nrf_error.h>
#include <simple_ble.h>
#include <eddystone.h>
#include <simple_adv.h>
//#include "multi_adv.h"

//tock includes
#include <signpost_api.h>
#include "tock.h"
#include "console.h"
#include "timer.h"
#include "iM880A_RadioInterface.h"
#include "i2c_master_slave.h"
#include "app_watchdog.h"
#include "radio_module.h"
#include "gpio.h"
#include "RadioDefs.h"

//definitions for the ble
#define DEVICE_NAME "Signpost"
#define PHYSWEB_URL "j2x.us/signp"


#define UMICH_COMPANY_IDENTIFIER 0x02E0

static simple_ble_config_t ble_config = {
    .platform_id        = 0x00,
    .device_id          = DEVICE_ID_DEFAULT,
    .adv_name           = (char *)DEVICE_NAME,
    .adv_interval       = MSEC_TO_UNITS(300, UNIT_0_625_MS),
    .min_conn_interval  = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval  = MSEC_TO_UNITS(1250, UNIT_1_25_MS),
};

//definitions for the i2c
#define BUFFER_SIZE 20
#define ADDRESS_SIZE 6
#define NUMBER_OF_MODULES 8

//i2c buffers
uint8_t slave_write_buf[BUFFER_SIZE];
uint8_t slave_read_buf[BUFFER_SIZE];
uint8_t master_read_buf[BUFFER_SIZE];
uint8_t master_write_buf[BUFFER_SIZE];

//array of the data we're going to send on the radios
uint8_t data_to_send[NUMBER_OF_MODULES][BUFFER_SIZE];
uint32_t lora_packets_sent = 1;
uint32_t lora_last_packets_sent = 0;

#ifndef COMPILE_TIME_ADDRESS
#error Missing required define COMPILE_TIME_ADDRESS of format: 0xC0, 0x98, 0xE5, 0x12, 0x00, 0x00
#endif
static uint8_t address[ADDRESS_SIZE] = { COMPILE_TIME_ADDRESS };


//these are the lora callback functions for seeing if everything is okay

static void lora_rx_callback(uint8_t* payload __attribute__ ((unused)),
                        uint8_t len __attribute__ ((unused)),
                        TRadioFlags flag __attribute__ ((unused))) {
    //this should never happen because I'm not receiving
//    putstr("Lora received a message?\n");
}

static void lora_tx_callback(TRadioMsg* message __attribute__ ((unused)),
                        uint8_t status) {
    //right now the  radio library ONLY implements txdone messages
    if(status == DEVMGMT_STATUS_OK) {
        lora_packets_sent++;
    } else {
        putstr("Lora error, resetting...");
        //app_watchdog_reset_app();
    }
}

static void adv_config_data(void) {
    static uint8_t i = 0;

    static ble_advdata_manuf_data_t mandata;

    if(data_to_send[i][0] != 0x00) {
        mandata.company_identifier = UMICH_COMPANY_IDENTIFIER;
        mandata.data.p_data = data_to_send[i];
        mandata.data.size = BUFFER_SIZE;

        simple_adv_manuf_data(&mandata);
    }

    i++;
    if(i >= NUMBER_OF_MODULES) {
        i = 0;
    }
}

static void networking_api_callback(uint8_t source_address,
        signbus_frame_type_t frame_type, __attribute ((unused)) signbus_api_type_t api_type,
        uint8_t message_type, size_t message_length, uint8_t* message) {

    if (frame_type == NotificationFrame || frame_type == CommandFrame) {
        if(message_type == NetworkingSend) {
            switch(source_address) {
            case 0x20:
                // controller
                if(message[0] == 0x01) {
                    // energy and status
                    data_to_send[0][0] = source_address;
                    data_to_send[0][1] = message[0];
                    data_to_send[0][2]++;
                    if(message_length-1 <= BUFFER_SIZE-3) {
                        memcpy(data_to_send[0]+3, message+1, message_length-1);
                    } else {
                        memcpy(data_to_send[0]+3, message+1, BUFFER_SIZE-3);
                    }
                } else if (message[0] == 0x02) {
                    data_to_send[1][0] = source_address;
                    data_to_send[1][1] = message[0];
                    data_to_send[1][2]++;
                    if(message_length-1 <= BUFFER_SIZE-3) {
                        memcpy(data_to_send[1]+3, message+1, message_length-1);
                    } else {
                        memcpy(data_to_send[1]+3, message+1, BUFFER_SIZE-3);
                    }
                } else {
                    //this shouldn't happen
                }
                break;
            case 0x31:
                // 15.4 scanner
                data_to_send[2][0] = source_address;
                data_to_send[2][1] = message[0];
                data_to_send[2][2]++;
                if(message_length-1 <= BUFFER_SIZE-3) {
                    memcpy(data_to_send[2]+3, message+1, message_length-1);
                } else {
                    memcpy(data_to_send[2]+3, message+1, BUFFER_SIZE-3);
                }
                break;
            case 0x32:
                // ambient sensing
                data_to_send[3][0] = source_address;
                data_to_send[3][1] = message[0];
                data_to_send[3][2]++;
                if(message_length-1 <= BUFFER_SIZE-3) {
                    memcpy(data_to_send[3]+3, message+1, message_length-1);
                } else {
                    memcpy(data_to_send[3]+3, message+1, BUFFER_SIZE-3);
                }
                break;
            case 0x33:
                // audio sensing
                data_to_send[4][0] = source_address;
                data_to_send[4][1] = message[0];
                data_to_send[4][2]++;
                if(message_length-1 <= BUFFER_SIZE-3) {
                    memcpy(data_to_send[4]+3, message+1, message_length-1);
                } else {
                    memcpy(data_to_send[4]+3, message+1, BUFFER_SIZE-3);
                }
                break;
            case 0x34:
                // radar module
                data_to_send[5][0] = source_address;
                data_to_send[5][1] = message[0];
                data_to_send[5][2]++;
                if(message_length-1 <= BUFFER_SIZE-3) {
                    memcpy(data_to_send[5]+3, message+1, message_length-1);
                } else {
                    memcpy(data_to_send[5]+3, message+1, BUFFER_SIZE-3);
                }
                break;
            case 0x35:
                // air quality sensing
                data_to_send[6][0] = source_address;
                data_to_send[6][1] = message[0];
                data_to_send[6][2]++;
                if(message_length-1 <= BUFFER_SIZE-3) {
                    memcpy(data_to_send[6]+3, message+1, message_length-1);
                } else {
                    memcpy(data_to_send[6]+3, message+1, BUFFER_SIZE-3);
                }
            default:
                //this shouldn't happen
                break;
            }
        }
    }

    //app_watchdog_tickle_kernel();
}

void ble_address_set(void) {
    static ble_gap_addr_t gap_addr;

    //switch the addres to little endian in a for loop
    for(uint8_t i = 0; i < ADDRESS_SIZE; i++) {
        gap_addr.addr[i] = address[ADDRESS_SIZE-1-i];
    }

    //set the address type
    gap_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
    uint32_t err_code = sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &gap_addr);
    APP_ERROR_CHECK(err_code);
}

void ble_error(uint32_t error_code __attribute__ ((unused))) {
    //this has to be here too
    putstr("ble error, resetting...");
    //app_watchdog_reset_app();
}

void ble_evt_connected(ble_evt_t* p_ble_evt __attribute__ ((unused))) {
    //this might also need to be here
}

void ble_evt_disconnected(ble_evt_t* p_ble_evt __attribute__ ((unused))) {
    //this too
}

void ble_evt_user_handler (ble_evt_t* p_ble_evt __attribute__ ((unused))) {
    //and maybe this
}

static void timer_callback (
    int callback_type __attribute__ ((unused)),
    int length __attribute__ ((unused)),
    int unused __attribute__ ((unused)),
    void * callback_args __attribute__ ((unused))) {

    static uint8_t i = 0;
    static uint8_t LoRa_send_buffer[ADDRESS_SIZE + BUFFER_SIZE];

    if(data_to_send[i][0] != 0x00) {

        //before we send this packet, make sure the last one completed
        if(lora_last_packets_sent == lora_packets_sent) {
            //error
            putstr("lora error! Reseting..\n");
            //app_watchdog_reset_app();
        } else {
            lora_last_packets_sent = lora_packets_sent;
        }

        //increment the packet counter for this slot
        // //don't meta count packets sent packets
        uint16_t packets = (uint16_t)((data_to_send[7][3+i*2+1]) + (data_to_send[7][3+i*2] << 8));
        packets++;
        data_to_send[7][3+i*2] = (uint8_t)((packets >> 8) & 0xff);
        data_to_send[7][3+i*2+1] = (uint8_t)(packets  & 0xff);
        data_to_send[7][2]++;

        //send the packet
        memcpy(LoRa_send_buffer, address, ADDRESS_SIZE);
        memcpy(LoRa_send_buffer+ADDRESS_SIZE, data_to_send[i], BUFFER_SIZE);
        uint16_t status = iM880A_SendRadioTelegram(LoRa_send_buffer,BUFFER_SIZE+ADDRESS_SIZE);

        //parse the HCI layer error codes
        if(status != 0) {
            //error
            putstr("lora error! Resetting...\n");
            //app_watchdog_reset_app();
        }
    }

    if(i == NUMBER_OF_MODULES-1) {
    //    eddystone_adv((char*)PHYSWEB_URL, NULL);
    } else {
     //   adv_config_data();
    }

    i++;
    if(i >= NUMBER_OF_MODULES) {
        i = 0;
    }
}

int main (void) {
    printf("starting app!\n");
    //do module initialization
    //I did it last because as soon as we init we will start getting callbacks
    //those callbacks depend on the setup above
    int rc;

    static api_handler_t networking_handler = {NetworkingApiType, networking_api_callback};
    static api_handler_t* handlers[] = {&networking_handler,NULL};
    do {
        rc = signpost_initialization_module_init(ModuleAddressRadio,handlers);
        if (rc<0) {
            delay_ms(5000);
        }
    } while (rc<0);

    gpio_enable_output(BLE_POWER);
    gpio_set(BLE_POWER);
    delay_ms(10);
    gpio_clear(BLE_POWER);

    //soft reset the LoRa Radio at startup
    gpio_enable_output(LORA_RESET);
    gpio_clear(LORA_RESET);
    delay_ms(50);
    gpio_set(LORA_RESET);

    //ble
    //simple_ble_init(&ble_config);

    //setup a tock timer to
    //eddystone_adv((char *)PHYSWEB_URL,NULL);

    //zero the rest of the data array
    for(uint8_t i = 0; i < NUMBER_OF_MODULES; i++) {
       for(uint8_t j = 0; j < BUFFER_SIZE; j++) {
           data_to_send[i][j] = 0;
        }
    }

    //init the one data slot that comes from us
    data_to_send[7][0] = 0x22;
    data_to_send[7][1] = 0x01;

    //setup lora
    //register radio callbacks
    iM880A_Init();
    iM880A_RegisterRadioCallbacks(lora_rx_callback, lora_tx_callback);
    //configure
    iM880A_Configure();

    // Setup a watchdog
    //app_watchdog_set_kernel_timeout(10000);
    //app_watchdog_start();

    //setup timer
    timer_subscribe(timer_callback, NULL);
    timer_start_repeating(2000);
}
