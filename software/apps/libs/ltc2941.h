#ifndef LTC2941_H
#define LTC2941_H


#define DRIVER_NUM_LTC2941 102

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	InterruptPinDisabled = 0,
	InterruptPinChargeCompleteMode = 1,
	InterruptPinAlertMode = 2,
} interrupt_pin_conf_e;

typedef enum {
	VbatAlertOff = 0,
	VbatAlert2V8 = 1,
	VbatAlert2V9 = 2,
	VbatAlert3V0 = 3,
} vbat_alert_e;


// Set a callback for the LTC2941 driver.
//
// The callback function should look like:
//
//     void callback (int callback_type, int data, int data2, void* callback_args)
//
// callback_type is one of:
//    0: If the interrupt pin is setup in the kernel, the interrupt occurred.
//    1: Got the contents of the status register. `data` is:
//           bit 0: Undervoltage lockout (bool)
//           bit 1: VBat Alert (bool)
//           bit 2: Charge Alert Low (bool)
//           bit 3: Charge Alert High (bool)
//           bit 4: Accumulated Charge Overflow/Underflow (bool)
//        and `data2` is the Chip type:
//           1 = LTC2941
//           2 = LTC2942
//     2: Got the charge value.
//     3: A write operation finished.
int ltc2941_set_callback (subscribe_cb callback, void* callback_args);

// Get the current value of the status register. The result will be returned
// in the callback.
int ltc2941_read_status(void);

// Setup the LTC2941 by configuring its !AL/CC pin, charge counting prescaler,
// and VBat alert threshold.
// Will trigger a `done` callback.
int ltc2941_configure(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat);

// Set the current accumulated charge register to 0.
// Will trigger a `done` callback.
int ltc2941_reset_charge(void);

// Configure the high charge threshold. This will be triggered when the
// accumulated charge is greater than this value.
// Will trigger a `done` callback when the register has been set.
int ltc2941_set_high_threshold(uint16_t threshold);

// Configure the low charge threshold. This will be triggered when the
// accumulated charge is lower than this value.
// Will trigger a `done` callback when the register has been set.
int ltc2941_set_low_threshold(uint16_t threshold);

// Read the current charge.
// Will be returned in the callback.
int ltc2941_get_charge(void);

// Put the LTC2941 in a low power state.
// Will trigger a `done` callback.
int ltc2941_shutdown(void);


//
// Synchronous Versions
//
int ltc2941_read_status_sync(void);
int ltc2941_configure_sync(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat);
int ltc2941_reset_charge_sync(void);
int ltc2941_set_high_threshold_sync(uint16_t threshold);
int ltc2941_set_low_threshold_sync(uint16_t threshold);
int ltc2941_get_charge_sync(void);
int ltc2941_shutdown_sync(void);

#ifdef __cplusplus
}
#endif

#endif
