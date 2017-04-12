# makefile for user application

TOCK_BOARD = audio_module

STACK_SIZE = 4096
APP_HEAP_SIZE = 4096

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

