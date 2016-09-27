# Makefile for building the tock kernel for the signpost controller

SLOAD=sload
SDB=$(TOCK_BUILD_DIR)/kernel.sdb
SDB_MAINTAINER=$(shell whoami)
SDB_VERSION=$(shell git show-ref -s HEAD)
SDB_NAME=firestormtest.rs
SDB_DESCRIPTION="An OS for the signpost controller"

OBJCOPY?=arm-none-eabi-objcopy
OBJDUMP?=arm-none-eabi-objdump
OBJDUMP_FLAGS+= --disassemble-all --source --disassembler-options=force-thumb -C --section-headers

JLINK=JLinkExe
JLINK_OPTIONS=-device ATSAM4LC8C -if swd -speed 1200 -AutoConnect 1
JLINK_SCRIPTS_DIR=jtag/

BOARD=firestormtest

all: target/target/release/$(BOARD) target/target/release/$(BOARD).lst

.PHONY: doc
doc:
	@cargo doc --release --target=../../tock/chips/sam4l/target.json

.PHONY: target/target/release/$(BOARD)
target/target/release/$(BOARD):
	@cargo build --release --target=../../tock/chips/sam4l/target.json

target/target/release/$(BOARD).sdb: target/target/release/$(BOARD)
	@tput bold ; echo "Packing SDB..." ; tput sgr0
	@$(SLOAD) pack -m "$(SDB_MAINTAINER)" -v "$(SDB_VERSION)" -n "$(SDB_NAME)" -d $(SDB_DESCRIPTION) -o $@ $<

target/target/release/$(BOARD).lst: target/target/release/$(BOARD)
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $^ > $@

target/target/release/$(BOARD).hex: target/target/release/$(BOARD)
	@$(OBJCOPY) -Oihex $^ $@

.PHONY: clean
clean:
	@cargo clean

# upload programs over uart with stormloader
.PHONY: program
program: target/target/release/$(BOARD).sdb
	$(SLOAD) flash $<

# upload kernel over JTAG
.PHONY: flash
flash: target/target/release/$(BOARD).hex
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-kernel.jlink

# special command for the sam4l. Flashes the stormloader bootloader onto
# 	the SAM4L so that sload can communicate with it
.PHONY: flash-bootloader
flash-bootloader: bootloader/bootloader.bin
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-bootloader.jlink

