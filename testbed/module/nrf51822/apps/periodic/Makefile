PROJECT_NAME = $(shell basename "$(realpath ./)")

APPLICATION_SRCS = $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += softdevice_handler.c
APPLICATION_SRCS += ble_advdata.c
APPLICATION_SRCS += ble_conn_params.c
APPLICATION_SRCS += app_timer.c
APPLICATION_SRCS += app_error.c

APPLICATION_SRCS += nrf_drv_common.c
APPLICATION_SRCS += nrf_drv_uart.c

APPLICATION_SRCS += simple_ble.c
APPLICATION_SRCS += simple_adv.c
APPLICATION_SRCS += led.c

LIBRARY_PATHS += ./ ../../platform
SOURCE_PATHS += ./ ../../platform

RAM_KB = 32
FLASH_KB = 256
SDK_VERSION = 11
SOFTDEVICE_MODEL = s130

NRF_BASE_PATH ?= ../../nrf5x-base/
include $(NRF_BASE_PATH)/make/Makefile
