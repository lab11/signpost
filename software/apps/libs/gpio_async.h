#pragma once

#include "tock.h"
#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif



int gpio_async_set_callback (subscribe_cb callback, void* callback_args);

int gpio_async_enable_output(uint32_t port, uint8_t pin);
int gpio_async_set(uint32_t port, uint8_t pin);
int gpio_async_clear(uint32_t port, uint8_t pin);
int gpio_async_toggle(uint32_t port, uint8_t pin);
int gpio_async_enable_input(uint32_t port, uint8_t pin, GPIO_InputMode_t pin_config);
int gpio_async_read(uint32_t port, uint8_t pin);
// int gpio_enable_interrupt(GPIO_Pin_t pin, GPIO_InputMode_t pin_config,
//     GPIO_InterruptMode_t irq_config);
// int gpio_disable_interrupt(GPIO_Pin_t pin);
// int gpio_disable(GPIO_Pin_t pin);
// int gpio_interrupt_callback(subscribe_cb callback, void* callback_args);

#ifdef __cplusplus
}
#endif
