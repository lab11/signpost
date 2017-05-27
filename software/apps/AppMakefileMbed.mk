#This makefile is a supporting makefile for mbed apps
#It sets up variables so that it can call the Linux-specific JLinkMakefile

OUTPUT_PATH = $(APP_DIR)/BUILD/$(TARGET)/$(TOOLCHAIN)/
OUTPUT_BIN :=  $(shell basename $(APP_DIR)).bin

CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SIGNPOST_APP_DIR := $(abspath $(CURRENT_DIR))

ifeq ($(TARGET),NUCLEO_L432KC)
FLASH_START_ADDRESS = 0x08000000
JLINK_DEVICE = STM32L432KC
else
$(error Platform not know. Please add flash start address to AppMakefileMbed.mk)
endif

MBED_CHECK := $(shell mbed --version 2> /dev/null)

all: 
ifndef MBED_CHECK
	$(error You must install the mbed-cli tools (try pip install mbed-cli))
else
	mbed compile -m $(TARGET) -t $(TOOLCHAIN)
endif

JLINK_OPTIONS = -device $(JLINK_DEVICE) -if swd -speed 1000

include $(SIGNPOST_APP_DIR)/AppMakefileMbed.mk.posix
