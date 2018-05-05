# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# files needed for this code
C_SRCS   := $(wildcard *.c) 
C_SRCS   := $(wildcard **/log2fix.c) 
#C_SRCS   += ./log2fix/log2fix.c
#C_SRCS   += ./kiss_fft/kiss_fft.c
#C_SRCS   += ./kiss_fft/tools/kiss_fftr.c
#C_SRCS   += ./gcwa/c/ridgeTracker.c
#C_SRCS   += ./gcwa/c/dynArray.c

INCLUDE_PATHS += . ./kiss_fft ./kiss_fft/tools ./log2fix ./gcwa/c

CFLAGS += -DFIXED_POINT=16 -std=c99
CFLAGS += -Wno-type-limits -Wno-sign-compare

TOCK_BOARD = audio_module

# include makefile settings that are shared between applications
include ../../AppMakefile.mk

