# Makefile for building the tock kernel for the signpost storage master

SLOAD=sload
SDB=$(TOCK_BUILD_DIR)/kernel.sdb
SDB_MAINTAINER=$(shell whoami)
SDB_VERSION=$(shell git show-ref -s HEAD)
SDB_NAME=storage_master.rs
SDB_DESCRIPTION="An OS for the signpost storage master"

SIZE?=arm-none-eabi-size
OBJCOPY?=arm-none-eabi-objcopy
OBJDUMP?=arm-none-eabi-objdump
OBJDUMP_FLAGS+= --disassemble-all --source --disassembler-options=force-thumb -C --section-headers
GDB=arm-none-eabi-gdb

JLINK=JLinkExe
JLINK_OPTIONS=-device ATSAM4LC8C -if swd -speed 1200 -AutoConnect 1
JLINK_SCRIPTS_DIR=jtag/
TERMINAL=gnome-terminal -e

all: target/sam4l/release/storage_master

.PHONY: doc
doc:
	@cargo doc --release --target=sam4l.json

.PHONY: target/sam4l/release/storage_master
target/sam4l/release/storage_master:
	@cargo build --release --target=sam4l.json
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > target/sam4l/release/storage_master.lst
	@$(SIZE) $@

.PHONY: target/sam4l/debug/storage_master
target/sam4l/debug/storage_master:
	@cargo build --target=sam4l.json
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > target/sam4l/debug/storage_master.lst
	@$(SIZE) $@


target/sam4l/release/storage_master.sdb: target/sam4l/release/storage_master
	@tput bold ; echo "Packing SDB..." ; tput sgr0
	@$(SLOAD) pack -m "$(SDB_MAINTAINER)" -v "$(SDB_VERSION)" -n "$(SDB_NAME)" -d $(SDB_DESCRIPTION) -o $@ $<

target/sam4l/release/storage_master.hex: target/sam4l/release/storage_master
	@$(OBJCOPY) -Oihex $^ $@

target/sam4l/debug/storage_master.hex: target/sam4l/debug/storage_master
	@$(OBJCOPY) -Oihex $^ $@

.PHONY: clean
clean:
	@cargo clean

.PHONY: debug
debug: target/sam4l/debug/storage_master

# upload programs over uart with stormloader
.PHONY: program
program: target/sam4l/release/storage_master.sdb
	$(SLOAD) flash $<

# upload kernel over JTAG
.PHONY: flash
flash: target/sam4l/release/storage_master.hex
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-kernel.jlink

.PHONY: flash-debug
flash-debug: target/sam4l/debug/storage_master.hex
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-debug.jlink

# special command for the sam4l. Flashes the stormloader bootloader onto
# 	the SAM4L so that sload can communicate with it
.PHONY: flash-bootloader
flash-bootloader: bootloader/bootloader.bin
	$(JLINK) $(JLINK_OPTIONS) $(JLINK_SCRIPTS_DIR)/flash-bootloader.jlink

startdebug: debug-gdbinit
	$(TERMINAL) "JLinkGDBServer $(JLINK_OPTIONS) -port 2331"
	sleep 1
	$(TERMINAL) "$(GDB) target/sam4l/release/storage_master"

debug-gdbinit:
	printf "target remote localhost:2331\nload\nmon reset\nbreak main\n" > .gdbinit

