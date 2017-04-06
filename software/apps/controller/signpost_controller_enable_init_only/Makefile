# makefile for user application

TOCK_BOARD = controller

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

STACK_SIZE=4096
APP_HEAP_SIZE=4096

# files needed for this code
C_SRCS   := main.c
INCLUDE_PATHS += .

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

