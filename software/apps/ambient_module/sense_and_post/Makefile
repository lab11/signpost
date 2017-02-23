# makefile for user application

TOCK_BOARD = ambient_module

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c)
INCLUDE_PATHS += .

STACK_SIZE = 8192

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

