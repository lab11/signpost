# Base folder definitions
TOCK_USERLAND_BASE_DIR ?= ../../../kernel/tock/userland
SIGNPOST_USERLAND_BASE_DIR ?= ../..
LIBNAME := mbedtls
$(LIBNAME)_DIR=$(SIGNPOST_USERLAND_BASE_DIR)/support/mbedtls/$(LIBNAME)

# Grab all source files
$(LIBNAME)_SRCS += $(wildcard $($(LIBNAME)_DIR)/library/*.c)

# Don't put the build directory in the submodule
$(LIBNAME)_BUILDDIR := $(SIGNPOST_USERLAND_BASE_DIR)/support/mbedtls/build

# include/mbedtls/bn_mul.h has one path that clobbers r9, which will give a
# bunch of `error: PIC register clobbered by 'r9' in 'asm'` - however, they
# happen to have an escape hatch to protect the frame pointer register r7
# (which we don't use) that also avoids the use of r9 (which we _do_ use), so
# we can just turn this on and everything's happy. Great!
CPPFLAGS += -DMULADDC_CANNOT_USE_R7

include $(TOCK_USERLAND_BASE_DIR)/TockLibrary.mk

