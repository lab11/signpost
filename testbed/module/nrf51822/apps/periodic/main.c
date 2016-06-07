/*
 * Demonstrate a simple module capable of communicating data with the controller
 * Doesn't respond to commands from controller
 *
 * Does not use BLE
 */

// Global libraries
#include <stdint.h>
#include <string.h>

// Nordic libraries
#include "ble_advdata.h"
#include "app_util_platform.h"
#include "nrf_drv_uart.h"

// nrf5x-base libraries
#include "simple_ble.h"
#include "simple_adv.h"
#include "led.h"

// Platform libraries
#include "module.h"


// LED pin number
// 20 on Atum or 25 on BLEES
#define LED0 20
// 18 on Atum or 13 on BLEES
#define ERR_LED 18

// Timer for sending data
APP_TIMER_DEF(timer_send);
#define SEND_INTERVAL APP_TIMER_TICKS(1000, 0)

// Intervals for advertising and connections
// Necessary for initialization, but unused
#define DEVICE_NAME "MODULE"
static simple_ble_config_t ble_config = {
    .platform_id       = 0x00,              // used as 4th octect in device BLE address
    .device_id         = DEVICE_ID_DEFAULT,
    .adv_name          = DEVICE_NAME,       // used in advertisements if there is room
    .adv_interval      = MSEC_TO_UNITS(500, UNIT_0_625_MS),
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS)
};

// UART storage
//#define BUS_RX_BUF_SIZE 1
//char bus_rx_buf[BUS_RX_BUF_SIZE];

void ble_error(uint32_t error_code) {
    led_init(ERR_LED);
    led_on(ERR_LED);
}

void timer_send_callback (void* context) {
    char* transmit_str = "(1,a,TEST)";
    nrf_drv_uart_tx((uint8_t*)transmit_str, strlen(transmit_str));
}

void uart_event_handler (nrf_drv_uart_event_t* event, void* context) {
    if (event->type == NRF_DRV_UART_EVT_TX_DONE) {
        // transmission complete
        led_toggle(LED0);
    } else if (event->type == NRF_DRV_UART_EVT_RX_DONE) {
        // buffer length worth of data received
        //nrf_drv_uart_rx(bus_rx_buf, BUS_RX_BUF_SIZE);
    } else if (event->type == NRF_DRV_UART_EVT_ERROR) {
        led_on(ERR_LED);
    }
}

int main(void) {

    // Setup chip
    simple_ble_init(&ble_config);
    led_init(LED0);

    // Initialze UART
    ret_code_t err_code;
    nrf_drv_uart_config_t uart_config = {
        .pseltxd =              BUS_TX_PIN,
        .pselrxd =              BUS_RX_PIN,
        .pselcts =              0,
        .pselrts =              0,
        .p_context =            NULL,
        .hwfc =                 NRF_UART_HWFC_DISABLED,
        .parity =               NRF_UART_PARITY_EXCLUDED,
        .baudrate =             NRF_UART_BAUDRATE_230400,
        .interrupt_priority =   APP_IRQ_PRIORITY_LOW,
    };
    err_code = nrf_drv_uart_init(&uart_config, uart_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable RX on UART
    //nrf_drv_uart_rx_enable();
    //nrf_drv_uart_rx(bus_rx_buf, BUS_RX_BUF_SIZE);

    // Create timer
    app_timer_create(&timer_send, APP_TIMER_MODE_REPEATED, timer_send_callback);
    app_timer_start(timer_send, SEND_INTERVAL, NULL);

    while (1) {
        power_manage();
    }
}

