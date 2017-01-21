#pragma once

#include <stdint.h>

#define NUM_MODULES 8
// 0    - controller
// 1    - mod0
// n+1  - modn

#define ECDH_KEY_LENGTH 32

typedef struct module_struct {
    uint8_t     addr;
    uint16_t    type;
    uint8_t     modules[NUM_MODULES];
    uint8_t     keys[NUM_MODULES][ECDH_KEY_LENGTH];
} module;

//int module_init(uint8_t addr);

