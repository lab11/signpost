$(BUILD_PLATFORM_DIR)/libsignpost_hil.rlib: $(call rwildcard,$(SRC_DIR)../../signpost_hil/src/,*.rs) $(BUILD_PLATFORM_DIR)/libcore.rlib $(BUILD_PLATFORM_DIR)/libmain.rlib $(BUILD_PLATFORM_DIR)/libcommon.rlib | $(BUILD_PLATFORM_DIR)
	@echo "Building $@"
	@$(RUSTC) $(RUSTC_FLAGS) --out-dir $(BUILD_PLATFORM_DIR) $(SRC_DIR)../../signpost_hil/src/lib.rs
