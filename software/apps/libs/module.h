#pragma once

#include <stdint.h>

#include "app.h"

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

typedef enum module_address {
    ModuleAddressController = 0x20,
    ModuleAddressStorage = 0x21,
    ModuleAddressRadio = 0x22,
} module_address_t;

//int module_init(uint8_t addr);

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

typedef struct __attribute__((packed)) energy_information {
    uint32_t    energy_limit_24h_mJ;
    uint32_t    energy_used_24h_mJ;
    uint16_t    current_limit_24h_mA;
    uint16_t    current_average_60s_mA;
    uint8_t     energy_limit_warning_threshold;
    uint8_t     energy_limit_critical_threshold;
} energy_information_t;

_Static_assert(sizeof(energy_information_t) == 14, "On-wire structure size");

int energy_query(energy_information_t* energy);
int energy_query_async(energy_information_t* energy, app_cb cb);

