#pragma once

#include "tock.h"

#ifdef __cplusplus
extern "C" {
#endif

int lps331ap_set_callback (subscribe_cb callback, void* callback_args);
int lps331ap_get_pressure ();

#ifdef __cplusplus
}
#endif
