#include "tock.h"
#include "gpio_async.h"

#define DRIVER_NUM_GPIO_ASYNC 100

#define CONCAT_PORT_PIN(port, pin) (((pin & 0xFF)<<8) | (port & 0xFF))
#define CONCAT_PORT_PIN_DATA(port, pin, data) (((data & 0xFFFF)<<16) | ((pin & 0xFF)<<8) | (port & 0xFF))



struct gpio_async_data {
  bool fired;
  int value;
  int callback_type;
};

static struct gpio_async_data result = { .fired = false };

// Internal callback for faking synchronous reads
static void gpio_async_cb(__attribute__ ((unused)) int callback_type,
                       __attribute__ ((unused)) int value,
                       __attribute__ ((unused)) int unused,
                       void* ud) {
  struct gpio_async_data* result = (struct gpio_async_data*) ud;
  result->callback_type = callback_type;
  result->value = value;
  result->fired = true;
}


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




int gpio_async_enable_output_sync(uint32_t port, uint8_t pin) {
    int err;
    result.fired = false;

    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_enable_output(port, pin);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int gpio_async_set_sync(uint32_t port, uint8_t pin) {
    int err;
    result.fired = false;


    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_set(port, pin);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int gpio_async_clear_sync(uint32_t port, uint8_t pin) {
    int err;
    result.fired = false;

    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_clear(port, pin);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int gpio_async_toggle_sync(uint32_t port, uint8_t pin) {
    int err;
    result.fired = false;

    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_toggle(port, pin);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int gpio_async_enable_input_sync(uint32_t port, uint8_t pin, GPIO_InputMode_t pin_config) {
    int err;
    result.fired = false;

    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_enable_input(port, pin, pin_config);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}

int gpio_async_read_sync(uint32_t port, uint8_t pin) {
    int err;
    result.fired = false;

    err = gpio_async_set_callback(gpio_async_cb, (void*) &result);
    if (err < 0) return err;

    err = gpio_async_read(port, pin);
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}
