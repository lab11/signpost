# makefile for user application

# What hardware platform is this application for?
TOCK_BOARD = audio_module

# These are the default values. You can override them if you need.
# STACK_SIZE = 4096
# APP_HEAP_SIZE = 4096

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c)
CXX_SRCS := $(wildcard *.cc) $(wildcard *.cpp) $(wildcard *.cxx)

# If your app requires additional search paths for header files, add them here
# INCLUDE_PATHS +=

# include makefile settings that are shared between applications
include ../AppMakefile.mk

