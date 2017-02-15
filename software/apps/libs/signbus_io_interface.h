#pragma once

#include <stdint.h>
#include <tock.h>
#include "signbus_app_layer.h"

//this is the first i2c messaging library!
//
//The MTU of the i2c bus is 256Bytes.
//We need packets
//Here's the proposed packet structure
//1:ver,2:total len, 3bit flag, remaind fragment offset, src

#ifdef __cplusplus
extern "C" {
#endif

// Defines for testing. These are arbitrary addresses.
#define SIGNBUS_TEST_SENDER_I2C_ADDRESS   0x32
#define SIGNBUS_TEST_RECEIVER_I2C_ADDRESS 0x18

//initialize
void signbus_io_init(uint8_t src);

//synchronous send
//returns size
uint32_t signbus_io_send(uint8_t dest, uint8_t* data, uint32_t len);

//synchronous receive
uint32_t signbus_io_recv(uint8_t* data, uint32_t len, uint8_t* src);

//async receive
int signbus_io_recv_async(app_cb callback, uint8_t* data, uint32_t len, uint8_t* src);

//set the read buffer
void signbus_io_set_read_buffer(uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif
