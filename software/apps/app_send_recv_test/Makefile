# makefile for user application

TOCK_BOARD = audio_module

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := main.c
INCLUDE_PATHS += .

# include makefile settings that are shared between applications
include ../AppMakefile.mk

