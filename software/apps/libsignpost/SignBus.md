SignBus
=======

SignBus describes the intra-module communication for the Signpost platform.

The SignBus message format is described in detail [here](../../docs/MessageFormat.md).

Most users will only interact with the top-level APIs provided in `signbus.h`

The SignBus implementation is hierarchical:

    signbus
      ↳ signbus_app_layer
          ↳ signbus_protocol_layer
              ↳ signbus_io_interface
                  ↳ <i2c driver syscalls>
