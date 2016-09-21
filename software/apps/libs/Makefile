MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(dir $(MKFILE_PATH))

TOCK_BASE_DIR = $(CUR_DIR)../../kernel/tock/
TOCK_USERLAND_BASE_DIR = $(CUR_DIR)../../kernel/tock/userland/
BUILDDIR ?= build/$(TOCK_ARCH)

TOOLCHAIN ?= arm-none-eabi

AS := $(TOOLCHAIN)-as
ASFLAGS += -mcpu=$(TOCK_ARCH) -mthumb

CC := $(TOOLCHAIN)-gcc
CFLAGS += -O3 -Wall -Werror -I$(CUR_DIR) -I$(TOCK_USERLAND_BASE_DIR)/libtock -g -mcpu=$(TOCK_ARCH) -mthumb
CFLAGS += \
	    -fdata-sections -ffunction-sections\
	    -Wall\
	    -Wextra\
	    -Wl,-gc-sections\
	    -g\
	    -fPIC\
	    -msingle-pic-base\
	    -mpic-register=r9\
	    -mno-pic-data-is-text-relative

AR := $(TOOLCHAIN)-ar

C_SRCS=$(wildcard *.c)
AS_SRCS=
OBJS := $(patsubst %.S,$(BUILDDIR)/%.o,$(AS_SRCS))
OBJS += $(patsubst %.c,$(BUILDDIR)/%.o,$(C_SRCS))

.PHONY: all
all: $(BUILDDIR)/libs.a

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o: %.S | $(BUILDDIR)
	$(AS) $(ASFLAGS) -c -o $@ $<

$(BUILDDIR)/libs.a: $(OBJS) | $(BUILDDIR)
	$(AR) rc $@ $^
	$(TOOLCHAIN)-ranlib $@

.PHONY: clean
clean:
	rm -Rf $(BUILDDIR)

