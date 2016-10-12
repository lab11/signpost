#pragma once

#include "tock.h"

#ifdef __cplusplus
extern "C" {
#endif

int si7021_set_callback (subscribe_cb callback, void* callback_args);
int si7021_get_temperature_humidity ();

#ifdef __cplusplus
}
#endif
