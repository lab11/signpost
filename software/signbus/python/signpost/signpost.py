#!/usr/bin/env python3

import signbus

#class Signpost():
#    def __init__(self, *,
#            source_address,
#            device='/dev/i2c-6',
#            ):
#        self._signbus = Signbus(
#                source_address=source_address,
#                device=device,
#                )

class EdisonApi():
    def __init__(self, *,
            signbus,
            ):
        self._signbus = signbus

    def send_read_handle(self, *,
            handle,
            ):
        raise NotImplementedError

    def send_read_rpc(self, *,
            ):
        raise NotImplementedError

