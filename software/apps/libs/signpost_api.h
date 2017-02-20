#ifndef SIGNPOST_API_H
#define SIGNPOST_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "signbus_app_layer.h"
#include "signbus_protocol_layer.h"

#define NUM_MODULES 8

typedef void (*signpost_api_callback_t)(uint8_t source_address,
        signbus_frame_type_t frame_type, signbus_api_type_t api_type, uint8_t message_type,
        size_t message_length, uint8_t* message);

typedef struct api_handler {
    signbus_api_type_t       api_type;
    signpost_api_callback_t  callback;
} api_handler_t;

// Generic method to respond to any API Command with an Error
// Callers MUST echo back the api_type and message_type of the bad message.
int signpost_api_error_reply(uint8_t destination_address,
        signbus_api_type_t api_type, uint8_t message_type);

// API layer send call
int signpost_api_send(uint8_t destination_address,
                      signbus_frame_type_t frame_type,
                      signbus_api_type_t api_type,
                      uint8_t message_type,
                      size_t message_length,
                      uint8_t* message);

/**************************************************************************/
/* INITIALIZATION API                                                     */
/**************************************************************************/

#define SIGNPOST_INITIALIZATION_NO_APIS NULL

typedef enum initialization_state {
    WaitPriv = 0,
    KeyExchange,
    Done,
} initialization_state_t;

enum initialization_message_type {
   InitializationKeyExchange = 0,
   InitializationGetMods,
} initialization_message_type_t;

typedef enum module_address {
    ModuleAddressController = 0x20,
    ModuleAddressStorage = 0x21,
    ModuleAddressRadio = 0x22,
} module_address_t;

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

// A special initialization routine for the controller module only.
int signpost_initialization_controller_module_init(api_handler_t** api_handlers);

// Send a key exchange request to another module
// Assumes controller has already isolated source and target
//
// params:
//  destination_address - The I2C address of the module to exchange keys with
int signpost_initialization_key_exchange_send(uint8_t destination_address);

// Send a response to a key exchange request
// Assumes controller has already isolated source and target
//
// params:
//  source_address  - The I2C address of the module that sent a key exchange request
//  ecdh_params     - The buffer of ecdh params sent in the InitializationKeyExchange message
//  len             - The length of data in ecdh_params
int signpost_initialization_key_exchange_respond(uint8_t source_address, uint8_t* ecdh_params, size_t len);

/**************************************************************************/
/* STORAGE API                                                            */
/**************************************************************************/

enum storage_message_type {
   StorageWriteMessage = 0,
};

typedef struct {
   uint8_t value[8];
} Storage_Record_t;

int signpost_storage_write (uint8_t* data, size_t len, Storage_Record_t* record_pointer);
int signpost_storage_write_reply (uint8_t destination_address, uint8_t* record_pointer);

/**************************************************************************/
/* NETWORKING API                                                         */
/**************************************************************************/
enum networking_message_type {
    NetworkingPostMessage = 0,
};


typedef struct {
   const char* header;
   const char* value;
} http_header;

typedef struct {
   uint8_t header_len;
   char* header;
   uint8_t value_len;
   char* value;
} http_response_header;

typedef struct {
    uint16_t status;
    uint16_t reason_len;
    char* reason;
    uint8_t num_headers;
    http_response_header* headers;
    uint16_t body_len;
    uint8_t* body;
} http_response;

typedef struct{
   uint8_t num_headers;
   http_header* headers;
   uint16_t body_len;
   const uint8_t* body;
} http_request;

int signpost_networking_post(const char* url, http_request request, http_response* response);
int signpost_networking_post_reply(uint8_t src_addr, uint8_t* response, uint16_t response_len);

/**************************************************************************/
/* PROCESSING API                                                         */
/**************************************************************************/

enum processing_return_type {
    ProcessingSuccess = 0,
    ProcessingNotExist = 1,
    ProcessingSizeError = 2,
    ProcessingCRCError = 3,
};

enum processing_message_type {
    ProcessingInitMessage = 0,
    ProcessingOneWayMessage = 1,
    ProcessingTwoWayMessage = 2,
};

//the edison path to the python module for servicing the rpc
int signpost_processing_init(const char* path);

int signpost_processing_oneway_send(uint8_t* buf, uint16_t len);
int signpost_processing_twoway_send(uint8_t* buf, uint16_t len);
int signpost_processing_twoway_receive(uint8_t* buf, uint16_t* len);

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

enum energy_message_type {
    EnergyQueryMessage = 0,
    EnergyLevelWarning24hMessage = 1,
    EnergyLevelCritical24hMessage = 2,
    EnergyCurrentWarning60sMessage = 3,
};

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
int signpost_energy_query_reply(uint8_t destination_address, signpost_energy_information_t* info);

/**************************************************************************/
/* TIME & LOCATION API                                                    */
/**************************************************************************/

typedef enum {
    TimeLocationGetTimeMessage = 0,
    TimeLocationGetLocationMessage = 1,
    TimeLocationGetTimeNextPpsMessage = 2,
} signpost_timelocation_message_type_e;

typedef struct __attribute__((packed)) {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
} signpost_timelocation_time_t;

typedef struct __attribute__((packed)) {
    uint32_t latitude;  // Latitude in microdegrees (divide by 10^6 to get degrees)
    uint32_t longitude; // Longitude in microdegrees
} signpost_timelocation_location_t;

int signpost_timelocation_get_time(signpost_timelocation_time_t* time);
int signpost_timelocation_get_location(signpost_timelocation_location_t* location);

int signpost_timelocation_get_time_reply(uint8_t destination_address, signpost_timelocation_time_t* time);
int signpost_timelocation_get_location_reply(uint8_t destination_address, signpost_timelocation_location_t* location);


/**************************************************************************/
/* EDISON API                                                             */
/**************************************************************************/

typedef enum {
    EdisonReadHandleMessage = 0,
    EdisonReadRPCMessage = 1,
} signpost_edison_message_type_e;

#ifdef __cplusplus
}
#endif

#endif
