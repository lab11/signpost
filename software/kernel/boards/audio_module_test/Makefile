# Makefile for building the tock kernel for the signpost audio_module_test

SLOAD=sload
SDB=$(TOCK_BUILD_DIR)/kernel.sdb
SDB_MAINTAINER=$(shell whoami)
SDB_VERSION=$(shell git show-ref -s HEAD)
SDB_NAME=audio_module_test.rs
SDB_DESCRIPTION="An OS for the signpost audio_module_test"

SIZE?=arm-none-eabi-size
OBJCOPY?=arm-none-eabi-objcopy
OBJDUMP?=arm-none-eabi-objdump
OBJDUMP_FLAGS+= --disassemble-all --source --disassembler-options=force-thumb -C --section-headers

JLINK=JLinkExe
JLINK_OPTIONS=-device ATSAM4LC8C -if swd -speed 1200 -AutoConnect 1
JLINK_SCRIPTS_DIR=jtag/

all: target/sam4l/release/audio_module_test

.PHONY: doc
doc:
	@cargo doc --release --target=sam4l.json

.PHONY: target/sam4l/release/audio_module_test
target/sam4l/release/audio_module_test:
	@cargo build --release --target=sam4l.json
	@$(SIZE) $@

.PHONY: target/sam4l/debug/audio_module_test
target/sam4l/debug/audio_module_test:
	@cargo build --target=sam4l.json
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > target/sam4l/debug/audio_module_test.lst
	@$(SIZE) $@


target/sam4l/release/audio_module_test.sdb: target/sam4l/release/audio_module_test
	@tput bold ; echo "Packing SDB..." ; tput sgr0
	@$(SLOAD) pack -m "$(SDB_MAINTAINER)" -v "$(SDB_VERSION)" -n "$(SDB_NAME)" -d $(SDB_DESCRIPTION) -o $@ $<

target/sam4l/release/audio_module_test.hex: target/sam4l/release/audio_module_test
	@$(OBJCOPY) -Oihex $^ $@

target/sam4l/debug/audio_module_test.hex: target/sam4l/debug/audio_module_test
	@$(OBJCOPY) -Oihex $^ $@

.PHONY: clean
clean:
	@cargo clean

.PHONY: debug
debug: target/sam4l/debug/audio_module_test

# upload programs over uart with stormloader
.PHONY: program
program: target/sam4l/release/audio_module_test.sdb
	$(SLOAD) flash $<

# upload kernel over JTAG
.PHONY: flash
flash: target/sam4l/release/audio_module_test.hex
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-kernel.jlink

.PHONY: flash-debug
flash-debug: target/sam4l/debug/audio_module_test.hex
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-debug.jlink

# special command for the sam4l. Flashes the stormloader bootloader onto
# 	the SAM4L so that sload can communicate with it
.PHONY: flash-bootloader
flash-bootloader: bootloader/bootloader.bin
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-bootloader.jlink

