# makefile for user application

# the current directory
APP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LIBS_DIR := ../libs/

# files needed for this code
APP_SRCS   += test_crypt_client.cpp
APP_SRCS   += test_crypt_server.cpp
APP_SRCS   += client_manager.cpp
APP_SRCS   += basic_codec.cpp
APP_SRCS   += framed_transport.cpp
APP_SRCS   += message_buffer.cpp
APP_SRCS   += erpc_client_setup.cpp
APP_SRCS   += erpc_port_stdlib.cpp
#APP_SRCS   += erpc_setup_serial.cpp
APP_SRCS   += erpc_setup_i2c_master_slave.cpp
#APP_SRCS   += serial_transport.cpp
APP_SRCS   += i2c_master_slave_transport.cpp
				
INCLUDE_PATHS += .
INCLUDE_PATHS += $(LIBS_DIR)erpc/erpc_c/infra/
INCLUDE_PATHS += $(LIBS_DIR)erpc/erpc_c/setup/
INCLUDE_PATHS += $(LIBS_DIR)erpc/erpc_c/transports/
INCLUDE_PATHS += $(LIBS_DIR)erpc/erpc_c/port/
INCLUDE_PATHS += $(LIBS_DIR)erpc/erpc_c/config/

SOURCE_PATHS += .
SOURCE_PATHS += $(LIBS_DIR)erpc/erpc_c/infra/
SOURCE_PATHS += $(LIBS_DIR)erpc/erpc_c/setup/
SOURCE_PATHS += $(LIBS_DIR)erpc/erpc_c/transports/
SOURCE_PATHS += $(LIBS_DIR)erpc/erpc_c/port/
SOURCE_PATHS += $(LIBS_DIR)erpc/erpc_c/config/

VPATH=$(SOURCE_PATHS)

C_SRCS   := $(wildcard *.c)

ERPC_SRCS := $(wildcard *.erpc)

CXX_SRCS  := $(notdir $(APP_SRCS))

TOCK_BOARD = controller

# include makefile settings that are shared between applications
include ../AppMakefile.mk

