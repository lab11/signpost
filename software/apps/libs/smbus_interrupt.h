#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_SMBUSINT 104

int smbus_interrupt_set_callback(subscribe_cb callback, void* callback_args);
int smbus_interrupt_issue_alert_response(void);

#ifdef __cplusplus
}
#endif
