LIBSIGNPOST_MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
LIBSIGNPOST_DIR := $(abspath $(dir $(LIBSIGNPOST_MAKEFILE_PATH)))

ifeq ($(LIBSIGNPOST_DIR),$(CURDIR))
$(error Do not run make from the libs directory)
endif

LIBSIGNPOST_BUILDDIR ?= $(LIBSIGNPOST_DIR)/build/$(TOCK_ARCH)

LIBSIGNPOST_C_SRCS=$(wildcard $(LIBSIGNPOST_DIR)/*.c)
LIBSIGNPOST_AS_SRCS=$(wildcard $(LIBSIGNPOST_DIR)/*.s)
LIBSIGNPOST_OBJS := $(patsubst $(LIBSIGNPOST_DIR)/%.s,$(LIBSIGNPOST_BUILDDIR)/%.o,$(LIBSIGNPOST_AS_SRCS))
LIBSIGNPOST_OBJS += $(patsubst $(LIBSIGNPOST_DIR)/%.c,$(LIBSIGNPOST_BUILDDIR)/%.o,$(LIBSIGNPOST_C_SRCS))

LIBMBEDTLS_C_SRCS= $(wildcard $(LIBMBEDTLS_LIB_DIR)*.c)
LIBMBEDTLS_AS_SRCS=$(wildcard $(LIBMBEDTLS_LIB_DIR)*.s)
LIBMBEDTLS_OBJS := $(patsubst $(LIBMBEDTLS_LIB_DIR)%.s,$(LIBSIGNPOST_BUILDDIR)/%.o,$(LIBMBEDTLS_AS_SRCS))
LIBMBEDTLS_OBJS += $(patsubst $(LIBMBEDTLS_LIB_DIR)%.c,$(LIBSIGNPOST_BUILDDIR)/%.o,$(LIBMBEDTLS_C_SRCS))

# LIBSIGNPOST rules
$(LIBSIGNPOST_BUILDDIR)/libsignpost.a: $(LIBSIGNPOST_OBJS) $(LIBMBEDTLS_OBJS) | $(LIBSIGNPOST_BUILDDIR)
	$(TRACE_AR)
	$(Q)$(TOOLCHAIN)-ar rc $@ $^
	$(Q)$(TOOLCHAIN)-ranlib $@

$(LIBSIGNPOST_BUILDDIR):
	$(Q)mkdir -p $@

$(LIBSIGNPOST_BUILDDIR)/%.o: $(LIBSIGNPOST_DIR)/%.c | $(LIBSIGNPOST_BUILDDIR)
	$(TRACE_DEP)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)@" -MT"$@" "$<"
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(LIBSIGNPOST_BUILDDIR)/%.o: $(LIBMBEDTLS_LIB_DIR)%.c | $(LIBSIGNPOST_BUILDDIR)
	$(TRACE_DEP)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$(@:.o=.d)@" -MT"$@" "$<"
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(LIBSIGNPOST_BUILDDIR)/%.o: %.S | $(LIBSIGNPOST_BUILDDIR)
	$(TRACE_AS)
	$(Q)$(AS) $(ASFLAGS) -c -o $@ $<

# LIBMBEDCRYPT rules
#$(LIBSIGNPOST_BUILDDIR)/libmbedcrypto.a: $(LIBMBEDTLS_OBJS) | $(LIBSIGNPOST_BUILDDIR)
#	$(TRACE_AR)
#	$(Q)$(TOOLCHAIN)-ar rc $@ $^
#	$(Q)$(TOOLCHAIN)-ranlib $@

print-%  : ; @echo $* = $($*)

# Include dependency rules for picking up header changes (by convention at bottom of makefile)
LIBSIGNPOST_OBJS_NO_ARCHIVES=$(filter %.o,$(LIBSIGNPOST_OBJS))
-include $(LIBSIGNPOST_OBJS_NO_ARCHIVES:.o=.d)

LIBMBEDTLS_OBJS_NO_ARCHIVES=$(filter %.o,$(LIBMBEDTLS_OBJS))
-include $(LIBMBEDTLS_OBJS_NO_ARCHIVES:.o=.d)
