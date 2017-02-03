# Top-level phony all
.PHONY: all
all:

# makefile with shared settings among user applications

TOCK_BOARD ?= controller
TOCK_ARCH = cortex-m4

CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

TOCK_USERLAND_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/userland/)
TOCK_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/)
BUILDDIR ?= $(abspath $(APP_DIR)/build/$(TOCK_ARCH))

# add platform-specific library files
LIBSIGNPOST_DIR = $(CURRENT_DIR)libs/
LIBSIGNPOST ?= $(LIBSIGNPOST_DIR)build/$(TOCK_ARCH)/libsignpost.a
OBJS += $(LIBSIGNPOST)

# add mbedtls crypto library
LIBMBEDTLS_LIB_DIR = $(CURRENT_DIR)libs/mbedtls/library/
LIBMBEDTLS_INC_DIR = $(CURRENT_DIR)libs/mbedtls/include/
#LIBMEDTLS ?= $(LIBSIGNPOST_DIR)build/$(TOCK_ARCH)/libmbedcrypto.a
#OBJS += $(LIBMBEDTLS)

INCLUDE_PATHS += $(LIBSIGNPOST_DIR) $(LIBMBEDTLS_INC_DIR)
INCLUDES = $(addprefix -I,$(INCLUDE_PATHS))
CPPFLAGS += $(INCLUDES)

# include the library makefile. Should pull in rules to rebuild libraries
# when needed
include $(LIBSIGNPOST_DIR)/Makefile

# include mbedtls library makefile
#include $(LIBMBEDTLS_LIB_DIR)Makefile

# include userland master makefile. Contains rules and flags for actually
# 	building the application
include $(TOCK_USERLAND_BASE_DIR)/Makefile


# add platform-specific headers
.PHONY: clean
clean::
	rm -Rf $(BUILDDIR)/
	rm -Rf $(LIBSIGNPOST_DIR)/build/
	rm -Rf $(TOCK_USERLAND_BASE_DIR)/libtock/build/

