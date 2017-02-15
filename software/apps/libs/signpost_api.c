#include <string.h>

#include "signbus_app_layer.h"
#include "signpost_api.h"
#include "tock.h"

/**************************************************************************/
/* INITIALIZATION API                                                     */
/**************************************************************************/

static struct module_struct {
    uint8_t             i2c_address;
    signbus_api_type_t* api_types;
    int8_t              api_type_to_module_address[HighestApiType+1];
    uint8_t             keys[NUM_MODULES][ECDH_KEY_LENGTH];
} module_info;

int signpost_initialization_module_init(
        uint8_t i2c_address,
        signbus_api_type_t* api_types) {
    //TODO: Actually contact the controller
    for (int i=0; i < NUM_MODULES; i++) {
        memset(module_info.keys[i], 0, ECDH_KEY_LENGTH);
    }

    module_info.i2c_address = i2c_address;
    module_info.api_types = api_types;

    // Some are well known, hard code for now:
    module_info.api_type_to_module_address[InitializationApiType] = ModuleAddressController;
    module_info.api_type_to_module_address[StorageApiType] = ModuleAddressStorage;
    module_info.api_type_to_module_address[NetworkingApiType] = -1; /* not supported */
    module_info.api_type_to_module_address[ProcessingApiType] = -1; /* not supported */
    module_info.api_type_to_module_address[EnergyApiType] = ModuleAddressController;
    module_info.api_type_to_module_address[TimeLocationApiType] = ModuleAddressController;

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
    signbus_app_send(ModuleAddressController, key,
            CommandFrame, EnergyApiType, EnergyQueryMessage,
            0, NULL);
    // XXX app_recv_async(cb, key,
    // Fix API, all should be args to callback not *'s

    return SUCCESS;
}

/**************************************************************************/
/* TIME & LOCATION API                                                    */
/**************************************************************************/

