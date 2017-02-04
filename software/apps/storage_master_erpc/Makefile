

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c)
INCLUDE_PATHS += .

TOCK_BOARD = storage_master

# include makefile settings that are shared between applications
include ../AppMakefile.mk

