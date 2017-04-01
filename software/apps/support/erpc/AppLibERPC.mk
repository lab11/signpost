# When apps are building, they may refer to some erpc headers.
# This file is included by the app build process to pick up these dirs.

ifndef SIGNPOST_USERLAND_BASE_DIR
  $(error SIGNPOST_USERLAND_BASE_DIR not defined)
endif

CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/erpc/erpc_c/infra
CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/erpc/erpc_c/setup/
CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/erpc/erpc_c/transports/
CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/erpc/erpc_c/port/
CPPFLAGS += -I$(SIGNPOST_USERLAND_BASE_DIR)/support/erpc/erpc/erpc_c/config/

