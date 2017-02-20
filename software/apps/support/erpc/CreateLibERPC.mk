# We want to inherit the build flags and build rules of the upstream Signpost
# environment (pic and whatnot), but control our own fate for what we build

ifndef TOCK_ARCH
  $(error No TOCK_ARCH)
endif

BUILDDIR := build/$(TOCK_ARCH)


# Specify default target
$(BUILDDIR)/liberpc.a:


# We leverage Make's VPATH to allow us to just specify the source files we need
# as if they were local. We list sources here by where they came from for clarity

# Grab Signpost build rules
include ../../AppMakefile.mk

# Clear out any variables we don't want
C_SRCS :=
CXX_SRCS :=
OBJS :=

VPATH += erpc/erpc_c/infra/
CXXFLAGS += -Ierpc/erpc_c/infra
CXX_SRCS += basic_codec.cpp
CXX_SRCS += client_manager.cpp
CXX_SRCS += framed_transport.cpp
CXX_SRCS += message_buffer.cpp

VPATH += erpc/erpc_c/setup/
CXXFLAGS += -Ierpc/erpc_c/setup/
CXX_SRCS += erpc_client_setup.cpp
CXX_SRCS += erpc_setup_i2c_master_slave.cpp

VPATH += erpc/erpc_c/transports/
CXXFLAGS += -Ierpc/erpc_c/transports/
CXX_SRCS += i2c_master_slave_transport.cpp

VPATH += erpc/erpc_c/port/
CXXFLAGS += -Ierpc/erpc_c/port/
CXX_SRCS += minimal_port_lab11.cpp

#VPATH += erpc/erpc_c/config/
CXXFLAGS += -Ierpc/erpc_c/config/

OBJS += $(patsubst %.cpp,$(BUILDDIR)/%.o,$(filter %.cpp, $(CXX_SRCS)))

# Now that all the needed variables are set, write our actual build rule
$(BUILDDIR)/liberpc.a: $(OBJS) | $(BUILDDIR)
	@echo $(OBJS)
	$(AR) rc $@ $^
	$(RANLIB) $@
