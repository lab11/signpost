#pragma once

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

int gpio_async_set_callback (subscribe_cb callback, void* callback_args);
int ltc2941_read_status();
int ltc2941_configure(interrupt_pin_conf_e int_pin, uint8_t prescaler, vbat_alert_e vbat);
int ltc2941_reset_charge();
int ltc2941_set_high_threshold(uint16_t threshold);
int ltc2941_set_low_threshold(uint16_t threshold);
int ltc2941_get_charge();
int ltc2941_shutdown();
