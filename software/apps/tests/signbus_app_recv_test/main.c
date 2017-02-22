#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rng.h>
#include <timer.h>
#include <tock.h>

#include "signbus_app_layer.h"
#include "signbus_protocol_layer.h"
#include "signbus_io_interface.h"
#include "signpost_entropy.h"

#define INTERVAL_IN_MS 2000


static uint8_t key[32];
static uint8_t* addr_to_key(uint8_t addr __attribute__((unused)) ) {
    memset(key, 0, 32);
    strcpy((char *) key, "this is a key");
    return key;
}



static uint8_t sender_address;
static signbus_frame_type_t frame_type;
static signbus_api_type_t api_type;
static uint8_t message_type;
static size_t message_length;
static uint8_t* message;
static uint8_t message_buffer[1024];

static uint8_t protocol_buffer[1024];

static bool recent_message = false;

static void cb(int len_or_rc) {
    if (len_or_rc < 0) {
        printf("Error rx'ing: %d\n", len_or_rc);
    } else {
        // Print data for this received message
        printf("from: %02x, frame_type: %02x, api_type: %02x, message_type: %02x, message_len: %d, %02x\n",
                sender_address, frame_type, api_type, message_type, message_length, message_length);
        if ((frame_type == NotificationFrame) && (api_type == 0xde) && (message_type == 0xad)) {
            printf("message: >>>%s<<<\n", message);
        } else {
            printf("unknown message: 0x");
            for (size_t i = 0; i < message_length; i++) {
                printf("%02x", message[i]);
            }
            printf("\n");
        }
    }

    // Tickle our pseduo-watchdog
    recent_message = true;

    // Listen for another message (re-using the same buffers)
    signbus_app_recv_async(
            cb,
            &sender_address,
            addr_to_key,
            &frame_type,
            &api_type,
            &message_type,
            &message_length,
            &message,
            1024,
            message_buffer
            );
}


int main(void) {
    printf("\n###\n\n\ntest app stack\n");

    printf("RECEIVER: Begin listening\n\n");

    signpost_entropy_init();
    signbus_io_init(SIGNBUS_TEST_RECEIVER_I2C_ADDRESS);
    signbus_protocol_setup_async(protocol_buffer, 1024);
    signbus_app_recv_async(
            cb,
            &sender_address,
            addr_to_key,
            &frame_type,
            &api_type,
            &message_type,
            &message_length,
            &message,
            1024,
            message_buffer
            );
    while(1) {
        delay_ms(INTERVAL_IN_MS);
        if (recent_message == false) {
            printf("RECEIVER: No message in %d ms\n", INTERVAL_IN_MS);
        } else {
            recent_message = false;
        }
    }
}
