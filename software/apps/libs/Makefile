LIBS_MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
LIBS_DIR := $(dir $(LIBS_MAKEFILE_PATH))

ifeq ($(LIBS_DIR),$(CURDIR)/)
$(error Do not run make from the libs directory)
endif

LIBS_BUILDDIR ?= $(LIBS_DIR)/build/$(TOCK_ARCH)

LIBS_C_SRCS=$(wildcard $(LIBS_DIR)*.c)
LIBS_AS_SRCS=$(wildcard $(LIBS_DIR)*.s)
LIBS_OBJS := $(patsubst $(LIBS_DIR)%.s,$(LIBS_BUILDDIR)/%.o,$(LIBS_AS_SRCS))
LIBS_OBJS += $(patsubst $(LIBS_DIR)%.c,$(LIBS_BUILDDIR)/%.o,$(LIBS_C_SRCS))

$(LIBS_BUILDDIR)/libs.a: $(LIBS_OBJS) | $(LIBS_BUILDDIR)
	$(TRACE_AR)
	$(Q)$(TOOLCHAIN)-ar rc $@ $^
	$(Q)$(TOOLCHAIN)-ranlib $@

#.PHONY: all
#all: $(BUILDDIR)/libs.a
#
$(LIBS_BUILDDIR):
	mkdir -p $@

$(LIBS_BUILDDIR)/%.o: $(LIBS_DIR)%.c | $(LIBS_BUILDDIR)
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(LIBS_BUILDDIR)/%.o: %.S | $(LIBS_BUILDDIR)
	$(TRACE_AS)
	$(Q)$(AS) $(ASFLAGS) -c -o $@ $<

#$(BUILDDIR)/libs.a: $(OBJS) | $(BUILDDIR)
#	$(AR) rc $@ $^
#	$(TOOLCHAIN)-ranlib $@

