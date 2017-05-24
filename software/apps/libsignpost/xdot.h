#pragma once

#include <stdint.h>

//8 byte AppEUI, 16 byte AppKey
int xdot_join_network(uint8_t* AppEUI, uint8_t* AppKey);

//dr >= 0 and dr<=4
int xdot_set_txdr(uint8_t dr);

//ack=0 || ack=1
int xdot_set_ack(uint8_t ack);

//sends binary buffer
int xdot_send(uint8_t* buf, uint8_t len);
