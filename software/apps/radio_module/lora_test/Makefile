# Makefile for user application

# Specify this directory relative to the current application.
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Which files to compile.
C_SRCS := $(wildcard *.c)

LIBRARY_PATHS += .

TOCK_USERLAND_BASE_DIR := ../../../kernel/tock/userland

# include makefile settings that are shared between applications
include ../../AppMakefile.mk
