#include <firestorm.h>
#include <tock.h>
#include <gpio_async.h>

#define DRIVER_NUM_GPIO_ASYNC 100

#define CONCAT_PORT_PIN(port, pin) (((pin & 0xFF)<<8) | (port & 0xFF))
#define CONCAT_PORT_PIN_DATA(port, pin, data) (((data & 0xFFFF)<<16) | ((pin & 0xFF)<<8) | (port & 0xFF))

int gpio_async_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_GPIO_ASYNC, 0, callback, callback_args);
}

int gpio_async_enable_output(uint32_t port, uint8_t pin) {
	return command(DRIVER_NUM_GPIO_ASYNC, 0, CONCAT_PORT_PIN(port, pin));
}

int gpio_async_set(uint32_t port, uint8_t pin) {
	return command(DRIVER_NUM_GPIO_ASYNC, 1, CONCAT_PORT_PIN(port, pin));
}

int gpio_async_clear(uint32_t port, uint8_t pin) {
	return command(DRIVER_NUM_GPIO_ASYNC, 2, CONCAT_PORT_PIN(port, pin));
}

int gpio_async_toggle(uint32_t port, uint8_t pin) {
	return command(DRIVER_NUM_GPIO_ASYNC, 3, CONCAT_PORT_PIN(port, pin));
}

int gpio_async_enable_input(uint32_t port, uint8_t pin, GPIO_InputMode_t pin_config) {
	return command(DRIVER_NUM_GPIO_ASYNC, 4, CONCAT_PORT_PIN_DATA(port, pin, pin_config));
}

int gpio_async_read(uint32_t port, uint8_t pin) {
	return command(DRIVER_NUM_GPIO_ASYNC, 5, CONCAT_PORT_PIN(port, pin));
}
