#pragma once

#include <stdint.h>

#include "signbus_app_layer.h"
#include "signbus_protocol_layer.h"

#define NUM_MODULES 8

/**************************************************************************/
/* INITIALIZATION API                                                     */
/**************************************************************************/

#define SIGNPOST_INITIALIZATION_NO_APIS NULL

typedef enum module_address {
    ModuleAddressController = 0x20,
    ModuleAddressStorage = 0x21,
    ModuleAddressRadio = 0x22,
} module_address_t;

typedef struct api_handler {
    signbus_api_type_t       api_type;
    signbus_app_callback_t*  callback;
} api_handler_t;

// Initialize this module.
// Must be called before any other signpost API methods.
//
// params:
//  i2c_address  - The I2C address of this calling module
//  api_handlers - Array of signpost APIs that this calling module implements
//                 The final element of this array MUST be NULL.
//                 This array MUST be static (pointer must be valid forever).
//                 Modules that implement no APIs MUST pass SIGNPOST_INITIALIZATION_NO_APIS.
int signpost_initialization_module_init(
        uint8_t i2c_address,
        api_handler_t** api_handlers);

/**************************************************************************/
/* STORAGE API                                                            */
/**************************************************************************/

/**************************************************************************/
/* NETWORKING API                                                         */
/**************************************************************************/

/**************************************************************************/
/* PROCESSING API                                                         */
/**************************************************************************/

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

typedef struct __attribute__((packed)) energy_information {
    uint32_t    energy_limit_24h_mJ;
    uint32_t    energy_used_24h_mJ;
    uint16_t    current_limit_60s_mA;
    uint16_t    current_average_60s_mA;
    uint8_t     energy_limit_warning_threshold;
    uint8_t     energy_limit_critical_threshold;
} signpost_energy_information_t;

_Static_assert(sizeof(signpost_energy_information_t) == 14, "On-wire structure size");

int signpost_energy_query(signpost_energy_information_t* energy);
int signpost_energy_query_async(signpost_energy_information_t* energy, signbus_app_callback_t cb);

/**************************************************************************/
/* TIME & LOCATION API                                                    */
/**************************************************************************/

