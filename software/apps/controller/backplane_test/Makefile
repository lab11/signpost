# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c)
INCLUDE_PATHS += .

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

