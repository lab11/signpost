#include <firestorm.h>
#include <tock.h>
#include <i2c_selector.h>

#define DRIVER_NUM_SMBUSINT 104

int smbus_interrupt_set_callback(subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_SMBUSINT, 0, callback, callback_args);
}

int smbus_interrupt_issue_alert_response() {
    return command(DRIVER_NUM_SMBUSINT, 0, 0);
}

