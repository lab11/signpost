#include <firestorm.h>
#include <tock.h>
#include <si7021.h>

int si7021_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(107, 0, callback, callback_args);
}

int si7021_get_temperature_humidity () {
    return command(107, 0, 0);
}
