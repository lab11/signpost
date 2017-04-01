#pragma once

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
__attribute__((warn_unused_result))
int signpost_api_error_reply(uint8_t destination_address,
        signbus_api_type_t api_type, uint8_t message_type);

// Convenience method that will repeatedly try to reply with failures and
// itself "cannot" fail (it simply eventually gives up).
//
// The `print_warnings` and `print_on_first_send` parameters control whether
// this method prints to alert that a failure occured, with the former
// controlling all prints, and the latter designed to allow callers to simply
// directly call this method without needing to report failure themselves.
void signpost_api_error_reply_repeating(uint8_t destination_address,
        signbus_api_type_t api_type, uint8_t message_type,
        bool print_warnings, bool print_on_first_send, unsigned tries);

// API layer send call
__attribute__((warn_unused_result))
int signpost_api_send(uint8_t destination_address,
                      signbus_frame_type_t frame_type,
                      signbus_api_type_t api_type,
                      uint8_t message_type,
                      size_t message_length,
                      uint8_t* message);

uint8_t* signpost_api_addr_to_key(uint8_t addr);
int signpost_api_addr_to_mod_num(uint8_t addr);

/**************************************************************************/
/* INITIALIZATION API                                                     */
/**************************************************************************/

#define SIGNPOST_INITIALIZATION_NO_APIS NULL

typedef enum initialization_state {
    Start = 0,
    Isolated,
    KeyExchange,
    Done,
} initialization_state_t;

typedef enum initialization_message_type {
   InitializationDeclare = 0,
   InitializationKeyExchange,
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
__attribute__((warn_unused_result))
int signpost_initialization_module_init(
        uint8_t i2c_address,
        api_handler_t** api_handlers);

// A special initialization routine for the controller module only.
__attribute__((warn_unused_result))
int signpost_initialization_controller_module_init(api_handler_t** api_handlers);

// A special initialization routine for the storage master module only.
__attribute__((warn_unused_result))
int signpost_initialization_storage_master_init(api_handler_t** api_handlers);

// Send a key exchange request to another module
// Assumes controller has already isolated source and target
//
// params:
//  destination_address - The I2C address of the module to exchange keys with
__attribute__((warn_unused_result))
int signpost_initialization_key_exchange_send(uint8_t destination_address);

// Send a response to a declare request
// Assumes controller has already isolated source
//
// params:
//  source_address  - The I2C address of the module that sent a declare request
//  module_number   - The module slot that is currently isolated
__attribute__((warn_unused_result))
int signpost_initialization_declare_respond(uint8_t source_address, uint8_t module_number);

// Send a response to a key exchange request
// Assumes controller has already isolated source and target
//
// params:
//  source_address  - The I2C address of the module that sent a key exchange request
//  ecdh_params     - The buffer of ecdh params sent in the InitializationKeyExchange message
//  len             - The length of data in ecdh_params
__attribute__((warn_unused_result))
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

// Write data to the Storage Master
//
// params:
//  data            - Data to write
//  len             - Length of data
//  record_pointer  - Pointer to record that will indicate location of written data
__attribute__((warn_unused_result))
int signpost_storage_write (uint8_t* data, size_t len, Storage_Record_t* record_pointer);

// Storage master response to write request
//
// params:
//  destination_address - Address to reply to
//  record_pointer      - Data at record
__attribute__((warn_unused_result))
int signpost_storage_write_reply (uint8_t destination_address, uint8_t* record_pointer);

/**************************************************************************/
/* NETWORKING API                                                         */
/**************************************************************************/
enum networking_message_type {
    NetworkingPostMessage = 0,
    NetworkingSend = 1
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

__attribute__((warn_unused_result))
int signpost_networking_post(const char* url, http_request request, http_response* response);
__attribute__((warn_unused_result))
int signpost_networking_send_bytes(uint8_t destination_address, uint8_t* data, uint16_t data_len);
void signpost_networking_post_reply(uint8_t src_addr, uint8_t* response, uint16_t response_len);

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
    ProcessingEdisonReadMessage = 3,
    ProcessingEdisonResponseMessage = 4,
};

// Initialize, provide the path to the python module used by the RPC
//
// params:
//  path    - linux-style path to location of python module to handle this
//  modules rpcs (e.g. /path/to/python/module.py)
__attribute__((warn_unused_result))
int signpost_processing_init(const char* path);

// Send an RPC with no expected response
//
// params:
//  buf - buffer containing RPC to send
//  len - length of buf
__attribute__((warn_unused_result))
int signpost_processing_oneway_send(uint8_t* buf, uint16_t len);

// Send an RPC with an expected response
//
// params:
//  buf - buffer containing RPC to send
//  len - length of buf
__attribute__((warn_unused_result))
int signpost_processing_twoway_send(uint8_t* buf, uint16_t len);

// Receive RPC response
//
// params:
//  buf - buffer to store result
//  len - length of buf
__attribute__((warn_unused_result))
int signpost_processing_twoway_receive(uint8_t* buf, uint16_t* len);

// Reply from Storage Master to RPC requesting module
//
// params:
//  src_addr        - address of module that originally requested the RPC
//  message_type    - type of RPC message
//  response        - RPC response from compute resource
//  response_len    - len of response
__attribute__((warn_unused_result))
int signpost_processing_reply(uint8_t src_addr, uint8_t message_type, uint8_t* response, uint16_t response_len);

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

enum energy_message_type {
    EnergyQueryMessage = 0,
    EnergyLevelWarning24hMessage = 1,
    EnergyLevelCritical24hMessage = 2,
    EnergyCurrentWarning60sMessage = 3,
    EnergyReportMessage = 4,
    EnergyDutyCycleMessage = 5,
};

typedef struct __attribute__((packed)) energy_information {
    uint32_t    energy_limit_mAh;
    uint16_t    current_average_mA;
    uint8_t     energy_limit_warning_threshold;
    uint8_t     energy_limit_critical_threshold;
} signpost_energy_information_t;

typedef struct __attribute__((packed)) energy_report_module {
    uint8_t module_address; //module i2c address
    uint8_t module_percent; //an integer percent 0-100 that  the module has used
} signpost_energy_report_module_t;

typedef struct __attribute__((packed)) energy_report {
    uint8_t num_reports;
    signpost_energy_report_module_t* reports;
} signpost_energy_report_t;

_Static_assert(sizeof(signpost_energy_information_t) == 8, "On-wire structure size");

// Query the controller for energy information
//
// params:
//  energy  - an energy_information_t struct to fill
__attribute__((warn_unused_result))
int signpost_energy_query(signpost_energy_information_t* energy);

// Tell the controller to turn me off then on again in X time
// params:
//  time - time in milliseconds to turn on again
int signpost_energy_duty_cycle(uint32_t time_ms);


// Tell the controller about modules who have used energy
// params: a struct of module addresses and energy percents of yours they have used
// This will distribute energy since the last report to the modules that have used
// that energy.
int signpost_energy_report(signpost_energy_report_t* report);

// Query the controller for energy information, asynchronously
//
// params:
//  energy  - an energy_information_t struct to fill
//  cb      - the callback to call when energy information is collected
__attribute__((warn_unused_result))
int signpost_energy_query_async(signpost_energy_information_t* energy, signbus_app_callback_t cb);

// Response from the controller to the requesting module
//
// params:
//  destination_address -   requesting address for this energy information
//  info                -   energy information
__attribute__((warn_unused_result))
int signpost_energy_query_reply(uint8_t destination_address, signpost_energy_information_t* info);

//response from controller to  requesting module
//this acks the percentages assigned to each module
//reports a module integer percent of 0 on failure
int signpost_energy_report_reply(uint8_t destination_address, signpost_energy_report_t* report_response);

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
    uint8_t  satellite_count;
} signpost_timelocation_time_t;

typedef struct __attribute__((packed)) {
    uint32_t latitude;  // Latitude in microdegrees (divide by 10^6 to get degrees)
    uint32_t longitude; // Longitude in microdegrees
    uint8_t  satellite_count;
} signpost_timelocation_location_t;

// Get time from controller
//
// params:
//  time     - signpost_timelocation_time_t struct to fill
__attribute__((warn_unused_result))
int signpost_timelocation_get_time(signpost_timelocation_time_t* time);

// Get location from controller
//
// params:
//  location - signpost_location_time_t struct to fill
__attribute__((warn_unused_result))
int signpost_timelocation_get_location(signpost_timelocation_location_t* location);

// Controller reply to time requesting module
//
// params:
//
//  destination_address - i2c address of requesting module
//  time                - signpost_timelocation_time_t struct to return
__attribute__((warn_unused_result))
int signpost_timelocation_get_time_reply(uint8_t destination_address, signpost_timelocation_time_t* time);

// Controller reply to location requesting module
//
// params:
//
//  destination_address - i2c address of requesting module
//  location            - signpost_timelocation_location_t struct to return
__attribute__((warn_unused_result))
int signpost_timelocation_get_location_reply(uint8_t destination_address, signpost_timelocation_location_t* location);

/**************************************************************************/
/* WATCHDOG API                                                           */
/**************************************************************************/

typedef enum {
    WatchdogStartMessage = 0,
    WatchdogTickleMessage = 1,
    WatchdogResponseMessage = 2,
} signpost_watchdog_message_type_e;

int signpost_watchdog_start(void);
int signpost_watchdog_tickle(void);
int signpost_watchdog_reply(uint8_t destination_address);

/**************************************************************************/
/* EDISON API                                                             */
/**************************************************************************/

typedef enum {
    EdisonReadHandleMessage = 0,
    EdisonReadRPCMessage = 1,
} signpost_edison_message_type_e;

/**************************************************************************/
/* JSON API                                                               */
/**************************************************************************/

typedef enum {
    JsonSend = 0,
} signpost_json_message_type_e;

typedef struct json_field {
    const char* name;
    int   value;
} json_field_t;

// Send JSON bytes
//
// params:
//  destination_address - i2c address of module to send JSON to
//  field_count         - number of JSON fields
//
//  any number of:
//  field_name          - c string indicating the name of the field
//  value               - integer value of the field
int signpost_json_send(uint8_t destination_address, size_t field_count, ... );

#ifdef __cplusplus
}
#endif
