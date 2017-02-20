# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

DEFINED_IF_APP_USES_ERPC=1

C_SRCS   := $(wildcard *.c)
ERPC_SRCS := $(wildcard *.erpc)

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

