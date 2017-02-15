#include "module.h"
#include "signbus_app_layer.h"
#include "tock.h"


/**************************************************************************/
/* ENERGY API                                                             */
/**************************************************************************/

typedef enum energy_message_type {
    EnergyQueryMessage = 0,
    EnergyLevelWarning24hMessage = 1,
    EnergyLevelCritical24hMessage = 2,
    EnergyCurrentWarning60sMessage = 3,
} energy_message_type_t;

static bool energy_query_ready;

static void energy_query_callback(
        size_t what __attribute__((unused))
        ) {
    energy_query_ready = true;
}

int energy_query(energy_information_t* energy) {
    energy_query_ready = false;

    {
        int rc = energy_query_async(energy, energy_query_callback);
        if (rc != 0) {
            return rc;
        }
    }

    yield_for(&energy_query_ready);

    return SUCCESS;
}

int energy_query_async(energy_information_t* energy, app_cb cb) {
    uint8_t* key = NULL;
    app_send(ModuleAddressController, key,
            CommandFrame, EnergyApiType, EnergyQueryMessage,
            0, NULL);
    // XXX app_recv_async(cb, key,
    // Fix API, all should be args to callback not *'s

    return SUCCESS;
}

/*
typedef void (app_cb)(size_t);

int app_send(uint8_t dest, uint8_t* key,
             frame_type_t frame_type, uint8_t api_type, uint8_t message_type,
             size_t message_length, uint8_t* message);


int app_recv(uint8_t* key,
        frame_type_t* frame_type, uint8_t* api_type, uint8_t* message_type,
        size_t* message_length, uint8_t* message);

int app_recv_async(app_cb cb, uint8_t* key,
                   frame_type_t* frame_type, uint8_t* api_type, uint8_t* message_type,
                   size_t* message_length, uint8_t* message);
*/
