SLOAD=sload
SDB=$(TOCK_BUILD_DIR)/kernel.sdb
SDB_MAINTAINER=$(shell whoami)
SDB_VERSION=$(shell git show-ref -s HEAD)
SDB_NAME=controller.rs
SDB_DESCRIPTION="An OS for the signpost controller"

all: target/target/release/controller

.PHONY: doc
doc:
	@cargo doc --release --target=../../chips/sam4l/target.json

.PHONY: target/target/release/controller
target/target/release/controller:
	@cargo build --release --target=../../tock/chips/sam4l/target.json

target/target/release/controller.sdb: target/target/release/controller
	@tput bold ; echo "Packing SDB..." ; tput sgr0
	@$(SLOAD) pack -m "$(SDB_MAINTAINER)" -v "$(SDB_VERSION)" -n "$(SDB_NAME)" -d $(SDB_DESCRIPTION) -o $@ $<

.PHONY: clean
clean:
	@cargo clean

.PHONY: program
program: target/target/release/controller.sdb
	$(SLOAD) flash $<

