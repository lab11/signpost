# Makefile for user application

# Specify this directory relative to the current application.
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Which files to compile.
C_SRCS := $(wildcard *.c)

ifneq ($(ID),)
ADDRESS ?= c098e51200$(ID)
endif
ifneq ($(ADDRESS),)
C_ADDRESS ?= $(shell python -c "print('0x' + ',0x'.join([\"$(ADDRESS)\"[i:i+2] for i in range(0, len(\"$(ADDRESS)\"), 2 )]))")
endif


# The trick here is to only include the rule to build .address if the
# requisite variable is define, otherwise we define a "build rule" that
# fails with a debug message

ifeq ($(C_ADDRESS),)
.address:
	@echo "You must supply an ID (last octet) or full address"
	@exit 1
else
.PHONY: force_address
.address: force_address
	$(Q)bash -c "diff <(echo $(C_ADDRESS)) .address &>/dev/null || echo $(C_ADDRESS) > .address"
endif

CFLAGS += -DCOMPILE_TIME_ADDRESS="$(C_ADDRESS)"

LIBRARY_PATHS += .

TOCK_USERLAND_BASE_DIR := ../../kernel/tock/userland

# Include the makefile for using BLE serialization.
include $(TOCK_USERLAND_BASE_DIR)/libnrfserialization/Makefile.app

# include makefile settings that are shared between applications
include ../AppMakefile.mk

$(BUILDDIR)/main.o: .address

