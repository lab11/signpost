# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c bonus_timer.c
INCLUDE_PATHS += .

# specific flags needed for this code
CPPFLAGS += -DSTACK_SIZE=2048

# include makefile settings that are shared between applications
include ../AppMakefile.mk

