# makefile for user application

APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

DEFINED_IF_APP_USES_ERPC=1

STACK_SIZE = 10000
APP_HEAP_SIZE = 8000

C_SRCS   := $(wildcard *.c)
ERPC_SRCS := $(wildcard *.erpc)

# include makefile settings that are shared between applications
include ../AppMakefile.mk

