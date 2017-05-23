# Makefile for building the tock kernel for the signpost radio module

CHIP=sam4l
PLATFORM=radio_module

include ../../../kernel/tock/boards/Makefile.common

# XXX This is a bit ugly, but is a nice feature
target/$(CHIP)/release/$(PLATFORM):	src/version.rs

.PHONY: FORCE
VCMD := echo \"pub static GIT_VERSION: &'static str = \\\"$$(git describe --always || echo notgit)\\\";\"
src/version.rs: FORCE
	@bash -c "cmp -s <($(VCMD)) <(test -e $@ && cat $@) || $(VCMD) > $@"

TOCKLOADER=tockloader

# Where in the SAM4L flash to load the kernel with `tockloader`
KERNEL_ADDRESS=0x10000

BOOTLOADER_FILE = ../bootloader/justjump_bootloader.bin

# Upload programs over uart with tockloader
ifdef PORT
  TOCKLOADER_GENERAL_FLAGS += --port $(PORT)
endif

TOCKLOADER_JTAG_FLAGS = --jtag --board $(PLATFORM) --arch cortex-m4 --jtag-device ATSAM4LC8C

# upload kernel over USB, unsupported
.PHONY: program
program: target/sam4l/release/$(PLATFORM).bin
	@echo "\nCannot program Signpost modules over USB. Use \`make flash\` and JTAG."

# upload kernel over JTAG
.PHONY: flash
flash: target/sam4l/release/$(PLATFORM).bin
	@$(MAKE) flash-kernel || ($(MAKE) flash-bootloader && $(MAKE) flash-kernel)

.PHONY: flash-kernel
flash-kernel: target/sam4l/release/$(PLATFORM).bin
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) flash --address $(KERNEL_ADDRESS) --jtag $<

.PHONY: flash-debug
flash-debug: target/sam4l/debug/$(PLATFORM).bin
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) flash --address $(KERNEL_ADDRESS) --jtag $<

# Command to flash the bootloader. Flashes the bootloader onto the SAM4L. Also
# sets the appropriate attributes.
.PHONY: flash-bootloader
flash-bootloader: $(BOOTLOADER_FILE)
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) flash --address 0 $(TOCKLOADER_JTAG_FLAGS) $<
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) set-attribute board    $(PLATFORM) $(TOCKLOADER_JTAG_FLAGS)
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) set-attribute arch     cortex-m4   $(TOCKLOADER_JTAG_FLAGS)
	@$(TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) set-attribute jldevice ATSAM4LC8C  $(TOCKLOADER_JTAG_FLAGS)

