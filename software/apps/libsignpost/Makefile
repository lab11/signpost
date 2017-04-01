# Libsignpost makefile. Can build libsignpost standalone. Also included by
# application makefiles to ensure their libsignpost dependency is built

# Base folder definitions
TOCK_USERLAND_BASE_DIR ?= ../../kernel/tock/userland
SIGNPOST_USERLAND_BASE_DIR ?= ..
LIBNAME := libsignpost
$(LIBNAME)_DIR=$(SIGNPOST_USERLAND_BASE_DIR)/$(LIBNAME)

# Grab all source files
$(LIBNAME)_SRCS += $(wildcard $($(LIBNAME)_DIR)/*.s)
$(LIBNAME)_SRCS += $(wildcard $($(LIBNAME)_DIR)/*.c)
$(LIBNAME)_SRCS += $(wildcard $($(LIBNAME)_DIR)/*.cc)
$(LIBNAME)_SRCS += $(wildcard $($(LIBNAME)_DIR)/*.cpp)

# External libraries
CPPFLAGS += -I$(TOCK_USERLAND_BASE_DIR)/libtock
CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/mbedtls/mbedtls/include
include $(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/AppLibERPC.mk

include $(TOCK_USERLAND_BASE_DIR)/TockLibrary.mk

