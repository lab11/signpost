# Makefile for building the tock kernel for the signpost storage master

PLATFORM=storage_master

# point to the signpost `boards/` directory
SIGNPOST_BOARDS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/../

include $(SIGNPOST_BOARDS_DIR)/Signpost-SAM4L-kernel.mk

