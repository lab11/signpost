#!/usr/bin/env python3

from . import app
from . import net
from . import protocol

class Signbus():
    def __init__(self, *,
            source_address,
            device='/dev/i2c-6'
            ):
        # net/link layer
        self._net = net.NetworkLayer(
                source_address=source_address,
                device=device,
                )
        # protocol layer
        self._protocol = protocol.ProtocolLayer(
                network_layer=self._net,
                )
        # application layer
        self._app = app.AppLayer(
                protocol_layer=self._protocol,
                )

        # Expose the application layer send as our send
        self.send = _app.send

