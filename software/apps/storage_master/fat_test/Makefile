# makefile for user application

TOCK_BOARD = storage_master

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c mmc_io.c
C_SRCS   += fatfs/ff.c fatfs/option/unicode.c

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

$(BUILDDIR)/%/:
	$(Q)mkdir -p $@

$(OBJS): | $(BUILDDIR)/fatfs/ $(BUILDDIR)/fatfs/option/

