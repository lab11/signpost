#pragma once

#include "tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_LPS331AP 106

int lps331ap_set_callback (subscribe_cb callback, void* callback_args);
int lps331ap_get_pressure (void);

int lps331ap_get_pressure_sync (void);

#ifdef __cplusplus
}
#endif
