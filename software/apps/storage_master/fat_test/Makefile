# makefile for user application

TOCK_BOARD = storage_master

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c mmc_io.c

# handle complex paths
vpath %.c fatfs
C_SRCS   += ff.c

vpath %.c fatfs/option
C_SRCS   += unicode.c

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

# To see the generated rules, run:
# $(info $(foreach arch,$(TOCK_ARCHS),$(call FATFS_RULES,$(arch))))
# Actually generate the rules for each architecture
$(foreach arch, $(TOCK_ARCHS), $(eval $(call FATFS_RULES,$(arch))))

