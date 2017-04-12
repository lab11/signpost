# makefile for user application

# don't care
#TOCK_BOARD = audio_module

STACK_SIZE = 8192
APP_HEAP_SIZE = 8192

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
CXX_SRCS   := main.cc
INCLUDE_PATHS += .

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

