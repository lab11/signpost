# Top-level phony all
.PHONY: all
all:

# set default stack and heap sizes for apps
APP_HEAP_SIZE ?= 8192
STACK_SIZE ?= 4096

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
include $(LIBSIGNPOST_DIR)/LibsMakefile.mk

# include mbedtls library makefile
#include $(LIBMBEDTLS_LIB_DIR)Makefile




# eRPC tool
ERPCGEN := $(CURRENT_DIR)support/erpc/bin/erpcgen

ifdef DEFINED_IF_APP_USES_ERPC
OBJS += $(CURRENT_DIR)support/erpc/liberpc/$(TOCK_ARCH)/liberpc.a
endif
include $(CURRENT_DIR)support/erpc/AppLibERPC.mk

ERPC_BUILDDIR := $(abspath build/erpc)
ERPC_CXX_SRCS += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%_client.cpp,$(ERPC_SRCS))
ERPC_CXX_SRCS += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%_server.cpp,$(ERPC_SRCS))
ERPC_H_SRCS   += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%.h,$(ERPC_SRCS))

# Note to self and future: **Cannot** use VPATH with built things.
#VPATH += $(ERPC_BUILDDIR)

$(ERPC_CXX_SRCS): | $(ERPC_BUILDDIR)

CPPFLAGS += -I$(ERPC_BUILDDIR)

ERPC_OBJS := $(patsubst $(ERPC_BUILDDIR)/%.cpp,$(BUILDDIR)/%.o,$(ERPC_CXX_SRCS))
OBJS += $(ERPC_OBJS)

$(ERPC_OBJS):	$(ERPC_H_SRCS) $(ERPC_CXX_SRCS)

# Sigh. I _really_ try to avoid copying these build rules around, but it'll
# have to do for now:
$(BUILDDIR)/%.o: $(ERPC_BUILDDIR)/%.cpp | $(BUILDDIR)
	$(TRACE_DEP)
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)@" -MT"$@" "$<"
	$(TRACE_CXX)
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(ERPC_BUILDDIR):
	@mkdir -p $(ERPC_BUILDDIR)

$(ERPC_H_SRCS): $(ERPC_BUILDDIR)/%.h: %.erpc	| $(ERPCGEN) $(ERPC_BUILDDIR)
	$(ERPCGEN) -o $(ERPC_BUILDDIR) $<

$(C_SRCS):	| $(ERPC_H_SRCS)

$(CXX_SRCS):	| $(ERPC_H_SRCS)



# XXX(Pat)
# Solve the absence of a path to TOCK_BOARD for now
CHIP_LAYOUT := $(CURRENT_DIR)/../kernel/boards/$(TOCK_BOARD)/chip_layout.ld

# include userland master makefile. Contains rules and flags for actually
# 	building the application
include $(TOCK_USERLAND_BASE_DIR)/Makefile

# XXX(Pat)
# Turn off some of the less critical warnings while we're developing heavily
CPPFLAGS += -Wno-suggest-attribute=pure -Wno-suggest-attribute=const
CPPFLAGS += -Wno-unused-macros

CFLAGS += -Wno-pointer-sign

CXXFLAGS += -Wno-suggest-override
CXXFLAGS += -Wno-suggest-final-methods
CXXFLAGS += -Wno-suggest-final-types


# add platform-specific headers
.PHONY: clean
clean::
	rm -Rf $(BUILDDIR)/
	rm -Rf $(LIBSIGNPOST_DIR)/build/
	rm -Rf $(TOCK_USERLAND_BASE_DIR)/libtock/build/


# include board-specific makefile in order to get `make flash` to work
include $(CURRENT_DIR)/../kernel/boards/$(TOCK_BOARD)/Makefile-app

