# makefile for user application

TOCK_BOARD = storage_master

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c
INCLUDE_PATHS += .

APP_HEAP_SIZE=2048
STACK_SIZE=4096

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

