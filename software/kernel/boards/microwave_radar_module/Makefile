# Makefile for building the tock kernel for the signpost microwave radar module

PLATFORM=microwave_radar_module

# point to the signpost `boards/` directory
SIGNPOST_BOARDS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/../

include $(SIGNPOST_BOARDS_DIR)/Signpost-SAM4L-kernel.mk

