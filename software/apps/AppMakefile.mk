# Top-level phony all
.PHONY: all
all:

# makefile with shared settings among user applications

TOCK_BOARD ?= controller
TOCK_ARCH = cortex-m4

# TODO(Pat) This should be thought about more
TOCK_BOARD_DIR ?= $(CURRENT_DIR)/../kernel/boards/$(TOCK_BOARD)

CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

TOCK_USERLAND_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/userland/)
TOCK_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/)
BUILDDIR ?= $(abspath $(APP_DIR)/build/$(TOCK_ARCH))

# create list of object files required
OBJS += $(patsubst %.c,$(BUILDDIR)/%.o,$(C_SRCS))

# add platform-specific library files
LIBS_DIR = $(CURRENT_DIR)/libs/
LIBS ?= $(LIBS_DIR)/build/$(TOCK_ARCH)/libs.a
OBJS += $(LIBS)

INCLUDE_PATHS += $(LIBS_DIR)
INCLUDES = $(addprefix -I,$(INCLUDE_PATHS))
CFLAGS += $(INCLUDES)

# include the library makefile. Should pull in rules to rebuild libraries
# when needed
include $(LIBS_DIR)/Makefile


# include userland master makefile. Contains rules and flags for actually
# 	building the application
include $(TOCK_USERLAND_BASE_DIR)/Makefile

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# add platform-specific headers
.PHONY: clean
clean::
	rm -Rf $(BUILDDIR)../
	rm -Rf $(LIBS_DIR)/build/
	rm -Rf $(TOCK_USERLAND_BASE_DIR)/libtock/build/

##   TODO(Pat) this include is handled by main tock makefile currently
##   # for programming individual apps, include platform app makefile
##   include ../../kernel/boards/$(TOCK_BOARD)/Makefile-app

