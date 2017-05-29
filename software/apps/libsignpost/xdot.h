//This is a driver for the Multitech XDOT. 
//http://www.multitech.com/brands/multiconnect-xdot
#pragma once

#include <stdint.h>

#define XDOT_SUCCESS 0
#define XDOT_ERROR -1
#define XDOT_NO_RESPONSE -2
#define XDOT_INVALID_PARAM -3
#define XDOT_MSG_TOO_LONG -4

//Initializes XDOT and turns of command echo.
int xdot_init(void);

//8 byte AppEUI, 16 byte AppKey
//Sends an OTA join request for this info
int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey);

//Sets TX Data Rate 0-4. This should adapt if Adaptive Data Rate is set
int xdot_set_txdr(uint8_t dr);

//dr >= 0 and dr<=4
//-1 on error
//Returns current data rate - this determines the maximum packet size
int xdot_get_txdr(void);

//0<=txpwr<=20
//Sets the TX Power in dbm. Max TX power is 20dbm.
int xdot_set_txpwr(uint8_t tx);

//0 or 1
//Turns on and off adaptive data rate.
int xdot_set_adr(uint8_t adr);

//ack=0-8
//Requires message acknowledgement from the gateway.
//If ack is 0, no ack is required.
//If ack is 1-8 it determines the number of retransmits
int xdot_set_ack(uint8_t ack);

//sends binary buffer
int xdot_send(uint8_t* buf, uint8_t len);

int xdot_reset(void);
int xdot_sleep(void);
int xdot_wake(void);
int xdot_save_settings(void);
