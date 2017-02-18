#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "signbus_app_layer.h"
#include "signbus_io_interface.h"
#include "signpost_api.h"
#include "signpost_entropy.h"
#include "signpost_mod_io.h"
#include "tock.h"
#include "gpio.h"
#include "timer.h"

static struct module_struct {
    uint8_t                 i2c_address;
    api_handler_t**         api_handlers;
    int8_t                  api_type_to_module_address[HighestApiType+1];
    uint8_t                 i2c_address_mods[NUM_MODULES];
    bool                    haskey[NUM_MODULES];
    uint8_t                 keys[NUM_MODULES][ECDH_KEY_LENGTH];
} module_info = {.i2c_address_mods = {ModuleAddressController, ModuleAddressStorage, ModuleAddressRadio}};


uint8_t* signpost_api_addr_to_key(uint8_t addr);

// Translate module address to pairwise key
uint8_t* signpost_api_addr_to_key(uint8_t addr) {
    for (size_t i = 0; i < NUM_MODULES; i++) {
        if (addr == module_info.i2c_address_mods[i] && module_info.haskey[i]) {
            uint8_t* key = module_info.keys[i];
            SIGNBUS_DEBUG("key: %p: 0x%02x%02x%02x...%02x\n", key,
                    key[0], key[1], key[2], key[ECDH_KEY_LENGTH-1]);
            return key;
        }
    }

    printf("WARN: Encryption key lookup for I2C address 0x%02x failed.\n", addr);
    printf("      This will likely result in a HMAC failure and a message drop.\n");

    SIGNBUS_DEBUG("key: NULL\n");
    return NULL;
}

int signpost_api_error_reply(uint8_t destination_address,
        signbus_api_type_t api_type, uint8_t message_type) {
    return signpost_api_send(destination_address,
            ErrorFrame, api_type, message_type, 0, NULL);
}

int signpost_api_send(uint8_t destination_address,
                      signbus_frame_type_t frame_type,
                      signbus_api_type_t api_type,
                      uint8_t message_type,
                      size_t message_length,
                      uint8_t* message) {

    return signbus_app_send(destination_address, signpost_api_addr_to_key, frame_type, api_type,
                            message_type, message_length, message);
}

/**************************************************************************/
/* INCOMING MESSAGE / ASYNCHRONOUS DISPATCH                                */
/**************************************************************************/

// We can only have one active receive call at a time, which means providing
// a synchronous-looking interface can be a little tricky, because we could
// be waiting for a synchronous reply when another module sends us a message

// To handle this, we have a single, shared receive mechanism that is always
// issuing an asynchronous receive.

#define INCOMING_MESSAGE_BUFFER_LENGTH 1024
static uint8_t               incoming_source_address;
static signbus_frame_type_t  incoming_frame_type;
static signbus_api_type_t    incoming_api_type;
static uint8_t               incoming_message_type;
static size_t                incoming_message_length;
static uint8_t*              incoming_message;
static uint8_t               incoming_message_buffer[INCOMING_MESSAGE_BUFFER_LENGTH];

// See comment in protocol_layer.h
static uint8_t               incoming_protocol_buffer[INCOMING_MESSAGE_BUFFER_LENGTH];

static signbus_app_callback_t* incoming_active_callback = NULL;



// Forward decl
static void signpost_api_recv_callback(int len_or_rc);

static void signpost_api_start_new_async_recv(void) {
    int rc = signbus_app_recv_async(signpost_api_recv_callback,
            &incoming_source_address, signpost_api_addr_to_key,
            &incoming_frame_type, &incoming_api_type,
            &incoming_message_type, &incoming_message_length, &incoming_message,
            INCOMING_MESSAGE_BUFFER_LENGTH, incoming_message_buffer);
    if (rc != 0) {
        printf("%s:%d UNKNOWN ERROR %d\n", __FILE__, __LINE__, rc);
        printf("*** NO MORE MESSAGES WILL BE RECEIVED ***\n");
    }
}

static void signpost_api_recv_callback(int len_or_rc) {
    SIGNBUS_DEBUG("len_or_rc %d\n", len_or_rc);

    if (len_or_rc < 0) {
        if (len_or_rc == -94) {
            // These return codes are a hack
            printf("Dropping message with HMAC/HASH failure\n");
            signpost_api_start_new_async_recv();
        } else {
            printf("%s:%d It's all fubar?\n", __FILE__, __LINE__);
            // XXX trip watchdog reset or s/t?
            return;
        }
    }

    if ( (incoming_frame_type == NotificationFrame) || (incoming_frame_type == CommandFrame) ) {
        api_handler_t** handler = module_info.api_handlers;
        while (handler != NULL) {
            if ((*handler)->api_type == incoming_api_type) {
                (*handler)->callback(incoming_source_address,
                        incoming_frame_type, incoming_api_type, incoming_message_type,
                        incoming_message_length, incoming_message);
                break;
            }
            handler++;
        }
        if (handler == NULL) {
            printf("Warn: Unsolicited message for api %d. Dropping\n", incoming_api_type);
        }
    } else if ( (incoming_frame_type == ResponseFrame) || (incoming_frame_type == ErrorFrame) ) {
        if (incoming_active_callback != NULL) {
            incoming_active_callback(len_or_rc);
        } else {
            printf("Warn: Unsolicited response/error. Dropping\n");
        }
    } else {
        printf("Invalid frame type: %d. Dropping message\n", incoming_frame_type);
    }

    signpost_api_start_new_async_recv();
}


/**************************************************************************/
/* INITIALIZATION API                                                     */
/**************************************************************************/

static void signpost_initialization_common(uint8_t i2c_address, api_handler_t** api_handlers) {
    SIGNBUS_DEBUG("i2c %02x handlers %p\n", i2c_address, api_handlers);

    int rc;

    // Initialize the lower layers
    signbus_io_init(i2c_address);
    rc = signpost_entropy_init();
    if (rc < 0) return rc;

    // See comment in protocol_layer.h
    signbus_protocol_setup_async(incoming_protocol_buffer, INCOMING_MESSAGE_BUFFER_LENGTH);

    // Clear keys
    for (int i=0; i < NUM_MODULES; i++) {
        module_info.haskey[i] = false;
        memset(module_info.keys[i], 0, ECDH_KEY_LENGTH);
    }

    // Save module configuration
    module_info.i2c_address = i2c_address;
    module_info.api_handlers = api_handlers;

    // Populate the well-known API types with fixed addresses
    module_info.api_type_to_module_address[InitializationApiType] = ModuleAddressController;
    module_info.api_type_to_module_address[StorageApiType] = ModuleAddressStorage;
    module_info.api_type_to_module_address[NetworkingApiType] = -1; /* not supported */
    module_info.api_type_to_module_address[ProcessingApiType] = -1; /* not supported */
    module_info.api_type_to_module_address[EnergyApiType] = ModuleAddressController;
    module_info.api_type_to_module_address[TimeLocationApiType] = ModuleAddressController;
}

int signpost_initialization_controller_module_init(api_handler_t** api_handlers) {
    signpost_initialization_common(ModuleAddressController, api_handlers);

    // HACK Put this here until this module init's correctly and reports its address to controller
    //      I happen to have my test board in module 6 using address 50.
    module_info.i2c_address_mods[6] = 0x50;

    // Begin listening for replies
    signpost_api_start_new_async_recv();

    SIGNBUS_DEBUG("complete\n");
    return 0;
}

int signpost_initialization_module_init(
        uint8_t i2c_address,
        api_handler_t** api_handlers) {
    signpost_initialization_common(i2c_address, api_handlers);

    //TODO: Contact the controller to fill in the module address array w/ non-hard coded

    // Begin listening for replies
    signpost_api_start_new_async_recv();

    SIGNBUS_DEBUG("complete\n");
    return 0;
}

/**************************************************************************/
/* STORAGE API                                                            */
/**************************************************************************/

/**************************************************************************/
/* NETWORKING API                                                         */
/**************************************************************************/

/**************************************************************************/
/* PROCESSING API                                                         */
/**************************************************************************/

static bool networking_ready;
static bool networking_result;
static void signpost_networking_post_callback(int result) {
    networking_ready = true;
    networking_result = result;
}

http_response signpost_networking_post(const char* url, http_request request) {

    //form the sending array
    uint16_t header_size = 0;
    for(uint8_t i = 0; i < request.num_headers; i++) {
        header_size += strlen(request.headers[i].header);
        header_size += strlen(request.headers[i].value);
        header_size += 2;
    }
    uint16_t message_size = request.body_len + strlen(url) + header_size + 6 + 25;

    //this is the max message size
    if(message_size > 4096) {
        //this is an error - we can't send this
        http_response none;
        return none;
    }

    uint8_t send[message_size];
    uint16_t send_index = 0;
    uint16_t len = strlen(url);
    send[0] = (len & 0x00ff);
    send[1] = ((len & 0xff00) >> 8);
    send_index += 2;
    memcpy(send+send_index,url,len);
    send_index += len;
    send[send_index] = request.num_headers;
    send_index++;
    bool has_content_length = false;
    for(uint8_t i = 0; i < request.num_headers; i++) {
        uint8_t f_len = strlen(request.headers[i].header);
        send[send_index] = f_len;
        send_index++;
        if(!strcasecmp(request.headers[i].header,"content-length")) {
            has_content_length = true;
        }
        memcpy(send+send_index,request.headers[i].header,f_len);
        send_index += f_len;
        f_len = strlen(request.headers[i].value);
        send[send_index] = f_len;
        send_index++;
        memcpy(send+send_index,request.headers[i].value,f_len);
        send_index += f_len;
    }
    len = request.body_len;

    //add a content length header if the sender doesn't
    if(!has_content_length) {
        uint8_t clen = strlen("content-length");
        send[send_index] = clen;
        send_index++;
        memcpy(send+send_index,(uint8_t*)"content-length",clen);
        send_index += clen;
        char* cbuf[5];
        sprintf(cbuf,"%d",len);
        clen = strlen(cbuf);
        send[send_index] = clen;
        send_index++;
        memcpy(send+send_index,cbuf,clen);
        send_index += clen;
    }

    send[send_index] = (len & 0x00ff);
    send[send_index + 1] = ((len & 0xff00) >> 8);
    send_index += 2;
    memcpy(send + send_index, request.body, request.body_len);
    send_index += len;

    //setup the response callback
    incoming_active_callback = signpost_networking_post_callback;
    networking_ready = false;

    //call app_send
    signbus_app_send(ModuleAddressRadio, signpost_api_addr_to_key, CommandFrame,
            NetworkingApiType, NetworkingPostMessage, send_index + 1, send);

    //wait for a response
    yield_for(&networking_ready);

    //parse the response

    http_response none;
    return none;
}

/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

static bool energy_query_ready;
static int  energy_query_result;
static signbus_app_callback_t* energy_cb = NULL;
static signpost_energy_information_t* energy_cb_data = NULL;

static void energy_query_sync_callback(int result) {
    SIGNBUS_DEBUG("result %d\n", result);
    energy_query_ready = true;
    energy_query_result = result;
}

int signpost_energy_query(signpost_energy_information_t* energy) {
    energy_query_ready = false;

    {
        int rc = signpost_energy_query_async(energy, energy_query_sync_callback);
        if (rc != 0) {
            return rc;
        }
    }

    yield_for(&energy_query_ready);

    return energy_query_result;
}

static void energy_query_async_callback(int len_or_rc) {
    SIGNBUS_DEBUG("len_or_rc %d\n", len_or_rc);

    if (len_or_rc != sizeof(signpost_energy_information_t)) {
        printf("%s:%d - Error: bad len, got %d, want %d\n",
                __FILE__, __LINE__, len_or_rc, sizeof(signpost_energy_information_t));
    } else {
        if (energy_cb_data != NULL) {
            memcpy(energy_cb_data, incoming_message, len_or_rc);
        }
        energy_cb_data = NULL;
    }

    if (energy_cb != NULL) {
        // allow recursion
        signbus_app_callback_t* temp = energy_cb;
        energy_cb = NULL;
        temp(len_or_rc);
    }
}

int signpost_energy_query_async(
        signpost_energy_information_t* energy,
        signbus_app_callback_t cb
        ) {
    if (incoming_active_callback != NULL) {
        // XXX: Consider multiplexing based on API
        return -1;
    }
    if (energy_cb != NULL) {
        return -1;
    }
    incoming_active_callback = energy_query_async_callback;
    energy_cb_data = energy;
    energy_cb = cb;

    int rc;
    rc = signbus_app_send(ModuleAddressController,
            signpost_api_addr_to_key,
            CommandFrame, EnergyApiType, EnergyQueryMessage,
            0, NULL);
    if (rc < 0) return rc;

    return SUCCESS;
}

int signpost_energy_query_reply(uint8_t destination_address,
        signpost_energy_information_t* info) {
    return signbus_app_send(destination_address,
            signpost_api_addr_to_key,
            ResponseFrame, EnergyApiType, EnergyQueryMessage,
            sizeof(signpost_energy_information_t), (uint8_t*) info);
}

/**************************************************************************/
/* TIME & LOCATION API                                                    */
/**************************************************************************/

