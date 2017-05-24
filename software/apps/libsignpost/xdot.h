#pragma once

#include <stdint.h>

#define XDOT_SUCCESS 0
#define XDOT_ERROR -1
#define XDOT_NO_RESPONSE -2
#define XDOT_INVALID_PARAM -3
#define XDOT_MSG_TOO_LONG -4

int xdot_init(void);

//8 byte AppEUI, 16 byte AppKey
int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey);

//dr >= 0 and dr<=4
int xdot_set_txdr(uint8_t dr);

//dr >= 0 and dr<=4
//-1 on error
int xdot_get_txdr(void);

//0<=txpwr<=20
int xdot_set_txpwr(uint8_t tx);

//0 or 1
int xdot_set_adr(uint8_t adr);

//ack=0 || ack=1
int xdot_set_ack(uint8_t ack);

//sends binary buffer
int xdot_send(uint8_t* buf, uint8_t len);

int xdot_reset(void);
int xdot_save_settings(void);
