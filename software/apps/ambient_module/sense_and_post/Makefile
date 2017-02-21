# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c)
INCLUDE_PATHS += .

# increase heap size as needed
APP_HEAP_SIZE=2048
STACK_SIZE=4096

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

