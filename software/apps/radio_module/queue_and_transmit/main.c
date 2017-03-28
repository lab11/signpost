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

/*static simple_ble_config_t ble_config = {
    .platform_id        = 0x00,
    .device_id          = DEVICE_ID_DEFAULT,
    .adv_name           = (char *)DEVICE_NAME,
    .adv_interval       = MSEC_TO_UNITS(300, UNIT_0_625_MS),
    .min_conn_interval  = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval  = MSEC_TO_UNITS(1250, UNIT_1_25_MS),
};*/

//definitions for the i2c
#define BUFFER_SIZE 21
#define ADDRESS_SIZE 6
#define NUMBER_OF_MODULES 8

//array of the data we're going to send on the radios
//make a queue of 30 deep
#define QUEUE_SIZE 30
uint8_t data_queue[QUEUE_SIZE][BUFFER_SIZE];
uint8_t queue_head = 0;
uint8_t queue_tail = 0;
uint32_t lora_packets_sent = 1;
uint32_t lora_last_packets_sent = 0;
uint8_t module_num_map[NUMBER_OF_MODULES] = {0};
uint8_t number_of_modules = 0;
uint8_t module_packet_count[NUMBER_OF_MODULES] = {0};
uint8_t status_send_buf[20] = {0};

static void increment_queue_pointer(uint8_t* p) {
    if(*p == (QUEUE_SIZE -1)) {
        *p = 0;
    } else {
        (*p)++;
    }
}

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

/*static void adv_config_data(void) {
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
}*/

static void count_module_packet(uint8_t module_address) {
    for(uint8_t i = 0; i < NUMBER_OF_MODULES; i++) {
        if(module_num_map[i] == 0) {
            module_num_map[i] = module_address;
            module_packet_count[i]++;
            number_of_modules++;
            break;
        } else if(module_num_map[i] == module_address) {
            module_packet_count[i]++;
            break;
        }
    }
}

static int8_t add_buffer_to_queue(uint8_t addr, uint8_t* buffer, uint8_t len) {
    uint8_t temp_tail = queue_tail;
    increment_queue_pointer(&temp_tail);
    if(temp_tail == queue_head) {
        return -1;
    } else {
        data_queue[queue_tail][0] = addr;
        if(len  <= BUFFER_SIZE -1) {
            memcpy(data_queue[queue_tail]+1, buffer, len);
        } else {
            memcpy(data_queue[queue_tail]+1, buffer, BUFFER_SIZE-1);
        }
        increment_queue_pointer(&queue_tail);
        return 0;
    }
}

static void networking_api_callback(uint8_t source_address,
        signbus_frame_type_t frame_type, __attribute ((unused)) signbus_api_type_t api_type,
        uint8_t message_type, size_t message_length, uint8_t* message) {

    if (frame_type == NotificationFrame || frame_type == CommandFrame) {
        if(message_type == NetworkingSend) {
            add_buffer_to_queue(source_address, message, message_length);
        }
    }
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

    static uint8_t LoRa_send_buffer[ADDRESS_SIZE + BUFFER_SIZE];
    static uint8_t send_counter = 0;

    if(queue_head != queue_tail) {

        if(lora_last_packets_sent == lora_packets_sent) {
            //error
            putstr("lora error! Reseting..\n");
            //app_watchdog_reset_app();
        } else {
            lora_last_packets_sent = lora_packets_sent;
        }

        //count the packet
        count_module_packet(data_queue[queue_head][0]);

        //send the packet
        memcpy(LoRa_send_buffer, address, ADDRESS_SIZE);
        memcpy(LoRa_send_buffer+ADDRESS_SIZE, data_queue[queue_head], BUFFER_SIZE);
        uint16_t status = iM880A_SendRadioTelegram(LoRa_send_buffer,BUFFER_SIZE+ADDRESS_SIZE);

        //parse the HCI layer error codes
        if(status != 0) {
            //error
            putstr("lora error! Resetting...\n");
            //app_watchdog_reset_app();
        }
        increment_queue_pointer(&queue_head);
    }

    send_counter++;

    //every minute put a status packet on the queue
    //also send an energy report to the controller
    if(send_counter == 30) {
        //increment the sequence number
        status_send_buf[1]++;
        status_send_buf[2] = number_of_modules;

        //copy the modules and their send numbers into the buffer
        uint8_t i = 0;
        for(; i < NUMBER_OF_MODULES; i++){
            if(module_num_map[i] != 0) {
                status_send_buf[3+ i*2] = module_num_map[i];
                status_send_buf[3+ i*2 + 1] = module_packet_count[i];
            } else {
                break;
            }
        }

        if(queue_tail >= queue_head) {
            status_send_buf[3+i*2] = queue_tail-queue_head;
        } else {
            status_send_buf[3+i*2] = QUEUE_SIZE-queue_head-queue_tail;
        }

        add_buffer_to_queue(0x22, status_send_buf, BUFFER_SIZE);

        //reset send_counter
        send_counter = 0;

        //reset_packet_send_bufs
        for(i = 0; i < NUMBER_OF_MODULES; i++) {
            module_packet_count[i] = 0;
        }
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

    status_send_buf[0] = 0x01;
    status_send_buf[1] = 0;
    //ble
    //simple_ble_init(&ble_config);

    //setup a tock timer to
    //eddystone_adv((char *)PHYSWEB_URL,NULL);

    for(uint8_t i = 0; i < QUEUE_SIZE; i++) {
        for(uint8_t j = 0; j < BUFFER_SIZE; j++) {
            data_queue[i][j] = 0;
        }
    }

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
