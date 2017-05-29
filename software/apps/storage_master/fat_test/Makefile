# makefile for user application

TOCK_BOARD = storage_master

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c mmc_io.c
C_SRCS   += fatfs/ff.c fatfs/option/unicode.c

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

# Rules to create the correct library build folder structure for a given architecture
# These will be used to create the different architecture versions of an app
# Argument $(1) is the Architecture (e.g. cortex-m0) to build for
#
# Note: all variables, other than $(1), used within this block must be double
# dollar-signed so that their values will be evaluated when run, not when
# generated
define FATFS_RULES

$$(BUILDDIR)/$(1)/fatfs/:
	$$(Q)mkdir -p $$@

$$(BUILDDIR)/$(1)/fatfs/option/:
	$$(Q)mkdir -p $$@

$$(OBJS_$(1)): | $$(BUILDDIR)/$(1)/fatfs/ $$(BUILDDIR)/$(1)/fatfs/option/
endef

# To see the generated rules, run:
# $(info $(foreach arch,$(TOCK_ARCHS),$(call FATFS_RULES,$(arch))))
# Actually generate the rules for each architecture
$(foreach arch, $(TOCK_ARCHS), $(eval $(call FATFS_RULES,$(arch))))

