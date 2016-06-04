#ifndef MODULE_H
#define MODULE_H

#ifndef DEVICE_NAME
#define DEVICE_NAME "MODULE"
#endif /*DEVICE_NAME*/

#include <stdint.h>

#define PLATFORM_ID_BYTE 0x00

// Address is written here in flash if the ID command is used
#define ADDRESS_FLASH_LOCATION 0x0003fff8

// UART for controller communication

#define BUS_RX_PIN 23
#define BUS_TX_PIN 24

#endif /*MODULE_H*/

