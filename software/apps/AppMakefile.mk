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
SIGNPOST_USERLAND_BASE_DIR := $(abspath $(CURRENT_DIR))
TOCK_USERLAND_BASE_DIR := $(abspath $(CURRENT_DIR)/../kernel/tock/userland/)

# Include the libsignpost makefile. Adds rules that will rebuild library when needed
include $(SIGNPOST_USERLAND_BASE_DIR)/libsignpost/Makefile
include $(SIGNPOST_USERLAND_BASE_DIR)/support/mbedtls/Makefile


## TODO: Needs adapting for TAB support
##
## # eRPC tool
## ERPCGEN := $(CURRENT_DIR)support/erpc/bin/erpcgen
## 
## ifdef DEFINED_IF_APP_USES_ERPC
## OBJS += $(CURRENT_DIR)support/erpc/liberpc/$(TOCK_ARCH)/liberpc.a
## endif
include $(CURRENT_DIR)support/erpc/AppLibERPC.mk
## 
## ERPC_BUILDDIR := $(abspath build/erpc)
## ERPC_CXX_SRCS += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%_client.cpp,$(ERPC_SRCS))
## ERPC_CXX_SRCS += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%_server.cpp,$(ERPC_SRCS))
## ERPC_H_SRCS   += $(patsubst %.erpc,$(ERPC_BUILDDIR)/%.h,$(ERPC_SRCS))
## 
## # Note to self and future: **Cannot** use VPATH with built things.
## #VPATH += $(ERPC_BUILDDIR)
## 
## $(ERPC_CXX_SRCS): | $(ERPC_BUILDDIR)
## 
## CPPFLAGS += -I$(ERPC_BUILDDIR)
## 
## ERPC_OBJS := $(patsubst $(ERPC_BUILDDIR)/%.cpp,$(BUILDDIR)/%.o,$(ERPC_CXX_SRCS))
## OBJS += $(ERPC_OBJS)
## 
## $(ERPC_OBJS):	$(ERPC_H_SRCS) $(ERPC_CXX_SRCS)
## 
## # Sigh. I _really_ try to avoid copying these build rules around, but it'll
## # have to do for now:
## #
## # XXX erpcgen-generated code generates some warnings :(
## ERPC_CPPFLAGS += -Wno-old-style-cast -Wno-zero-as-null-pointer-constant
## #
## $(BUILDDIR)/%.o: $(ERPC_BUILDDIR)/%.cpp | $(BUILDDIR)
## 	$(TRACE_DEP)
## 	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(ERPC_CPPFLAGS) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)@" -MT"$@" "$<"
## 	$(TRACE_CXX)
## 	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(ERPC_CPPFLAGS) -c -o $@ $<
## 
## $(ERPC_BUILDDIR):
## 	@mkdir -p $(ERPC_BUILDDIR)
## 
## $(ERPC_H_SRCS): $(ERPC_BUILDDIR)/%.h: %.erpc	| $(ERPCGEN) $(ERPC_BUILDDIR)
## 	$(ERPCGEN) -o $(ERPC_BUILDDIR) $<
## 
## $(C_SRCS):	| $(ERPC_H_SRCS)
## 
## $(CXX_SRCS):	| $(ERPC_H_SRCS)



# include userland master makefile. Contains rules and flags for actually
# 	building the application
TOCK_KERNEL_ROOT := $(SIGNPOST_USERLAND_BASE_DIR)/../kernel
include $(TOCK_USERLAND_BASE_DIR)/AppMakefile.mk

# XXX(Pat)
# Turn off some of the less critical warnings while we're developing heavily
CPPFLAGS += -Wno-suggest-attribute=pure -Wno-suggest-attribute=const
CPPFLAGS += -Wno-unused-macros

CFLAGS += -Wno-pointer-sign

CXXFLAGS += -Wno-suggest-override
CXXFLAGS += -Wno-suggest-final-methods
CXXFLAGS += -Wno-suggest-final-types

