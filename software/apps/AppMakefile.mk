# makefile with shared settings among user applications

TOCK_BOARD = controller
TOCK_ARCH = cortex-m4

CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#XXX: bring these back once PR #115 is in
#TOCK_USERLAND_BASE_DIR = $(CURRENT_DIR)/../kernel/tock/userland/
#TOCK_BASE_DIR = $(CURRENT_DIR)/../kernel/tock/
TOCK_BASE_DIR = $(CURRENT_DIR)/../kernel/tock/userland/
BUILDDIR ?= $(APP_DIR)/build/$(TOCK_ARCH)

# create list of object files required
OBJS += $(patsubst %.c,$(BUILDDIR)/%.o,$(C_SRCS))

# add platform-specific library files
LIBS_DIR = $(CURRENT_DIR)/libs/
LIBS ?= $(LIBS_DIR)/build/$(TOCK_ARCH)/libs.a
OBJS += $(LIBS)

INCLUDE_PATHS += $(LIBS_DIR)
INCLUDES = $(addprefix -I,$(INCLUDE_PATHS))
CFLAGS += $(INCLUDES)

# include userland master makefile. Contains rules and flags for actually
# 	building the application
#XXX: here too
#include $(TOCK_USERLAND_BASE_DIR)/Makefile
include $(TOCK_BASE_DIR)/Makefile

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(LIBS):
	make -C $(LIBS_DIR) TOCK_ARCH=$(TOCK_ARCH)

# add platform-specific headers
.PHONY:
clean:
	rm -Rf $(BUILDDIR)../
	rm -Rf $(LIBS_DIR)/build/

# for programming individual apps, include platform app makefile
include ../../kernel/boards/controller/Makefile-app

