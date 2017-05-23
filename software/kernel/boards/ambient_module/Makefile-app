# Makefile for loading applications for tockloader-compliant signpost modules

$(call check_defined, BUILDDIR)
$(call check_defined, PACKAGE_NAME)

APP_TOCKLOADER ?= tockloader

# Upload programs over UART with tockloader
ifdef PORT
  TOCKLOADER_GENERAL_FLAGS += --port $(PORT)
endif

.PHONY: program
program: $(BUILDDIR)/$(PACKAGE_NAME).tab
	@echo "\n Cannot program Signpost modules over USB. Use \`make flash\` and JTAG."

# Upload programs over JTAG
.PHONY: flash
flash: $(BUILDDIR)/$(PACKAGE_NAME).tab
	$(APP_TOCKLOADER) $(TOCKLOADER_GENERAL_FLAGS) install --jtag $<

