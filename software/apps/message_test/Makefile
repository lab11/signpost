# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LIBS_DIR := ../libs/

# files needed for this code
				
INCLUDE_PATHS += .

SOURCE_PATHS += .

VPATH=$(SOURCE_PATHS)

C_SRCS   := $(wildcard *.c)
#C_SRCS   += i2c_master_slave.c

CXX_SRCS  := $(notdir $(APP_SRCS))

# specific flags needed for this code
CPPFLAGS += -DSTACK_SIZE=2048

TOCK_BOARD = controller

# include makefile settings that are shared between applications
include ../AppMakefile.mk

