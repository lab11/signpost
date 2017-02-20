# Top-level phony all
.PHONY: all
all:

# makefile with shared settings among user applications

TOCK_BOARD ?= controller
TOCK_ARCH := cortex-m4

CURRENT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

SIGNPOST_BASE_DIR := $(abspath $(CURRENT_DIR))

TOCK_USERLAND_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/userland/)
TOCK_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/)
BUILDDIR ?= $(abspath $(APP_DIR)/build/$(TOCK_ARCH))

# add platform-specific library files
LIBSIGNPOST_DIR = $(CURRENT_DIR)libs
LIBSIGNPOST ?= $(LIBSIGNPOST_DIR)/build/$(TOCK_ARCH)/libsignpost.a
OBJS += $(LIBSIGNPOST)

# add mbedtls crypto library
LIBMBEDTLS_LIB_DIR = $(CURRENT_DIR)libs/mbedtls/library/
LIBMBEDTLS_INC_DIR = $(CURRENT_DIR)libs/mbedtls/include/
#LIBMEDTLS ?= $(LIBSIGNPOST_DIR)/build/$(TOCK_ARCH)/libmbedcrypto.a
#OBJS += $(LIBMBEDTLS)

INCLUDE_PATHS += $(LIBSIGNPOST_DIR) $(LIBMBEDTLS_INC_DIR)
INCLUDES = $(addprefix -I,$(INCLUDE_PATHS))
CPPFLAGS += $(INCLUDES)

# include the library makefile. Should pull in rules to rebuild libraries
# when needed
include $(LIBSIGNPOST_DIR)/Makefile

# include mbedtls library makefile
#include $(LIBMBEDTLS_LIB_DIR)Makefile




# eRPC tool
ERPCGEN := $(CURRENT_DIR)support/erpc/bin/erpcgen
OBJS += $(CURRENT_DIR)support/erpc/liberpc/$(TOCK_ARCH)/liberpc.a
include $(CURRENT_DIR)support/erpc/AppLibERPC.mk

ERPC_BUILDDIR := build/erpc
ERPC_CXX_SRCS += $(patsubst %.erpc,%_client.cpp,$(ERPC_SRCS))
ERPC_CXX_SRCS += $(patsubst %.erpc,%_server.cpp,$(ERPC_SRCS))
ERPC_H_SRCS   += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%.h,$(ERPC_SRCS))

VPATH += $(ERPC_BUILDDIR)

$(ERPC_CXX_SRCS): | $(ERPC_BUILDDIR)

CPPFLAGS += -I$(ERPC_BUILDDIR)
CXX_SRCS += $(ERPC_CXX_SRCS)

$(ERPC_BUILDDIR):
	@mkdir -p $(ERPC_BUILDDIR)

$(ERPC_H_SRCS): $(ERPC_BUILDDIR)/%.h: %.erpc	| $(ERPCGEN)
	$(ERPCGEN) -o $(ERPC_BUILDDIR) $<

$(C_SRCS):	| $(ERPC_H_SRCS)

$(CXX_SRCS):	| $(ERPC_H_SRCS)



# include userland master makefile. Contains rules and flags for actually
# 	building the application
include $(TOCK_USERLAND_BASE_DIR)/Makefile

# XXX(Pat)
# Turn off some of the less critical warnings while we're developing heavily
CPPFLAGS += -Wno-suggest-attribute=pure -Wno-suggest-attribute=const
CPPFLAGS += -Wno-unused-macros


# add platform-specific headers
.PHONY: clean
clean::
	rm -Rf $(BUILDDIR)/
	rm -Rf $(LIBSIGNPOST_DIR)/build/
	rm -Rf $(TOCK_USERLAND_BASE_DIR)/libtock/build/


# include board-specific makefile in order to get `make flash` to work
include $(CURRENT_DIR)/../kernel/boards/$(TOCK_BOARD)/Makefile-app

