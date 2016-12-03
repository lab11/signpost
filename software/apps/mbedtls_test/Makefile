# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

APP_SRCS += md.c
APP_SRCS += md_wrap.c
APP_SRCS += md2.c
APP_SRCS += md4.c
APP_SRCS += md5.c
APP_SRCS += sha1.c
APP_SRCS += sha256.c
APP_SRCS += sha512.c
APP_SRCS += ripemd160.c
APP_SRCS += aes.c

# files needed for this code
INCLUDE_PATHS += .
INCLUDE_PATHS += ../libs/mbedtls/include/

SOURCE_PATHS += .
SOURCE_PATHS += ../libs/mbedtls/library/

VPATH=$(SOURCE_PATHS)

C_SRCS   := $(wildcard *.c)
C_SRCS   += $(notdir $(APP_SRCS))

# specific flags needed for this code
CPPFLAGS += -DSTACK_SIZE=2048

# include makefile settings that are shared between applications
include ../AppMakefile.mk

