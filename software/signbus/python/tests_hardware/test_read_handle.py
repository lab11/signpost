#!/usr/bin/env python3

import logging
logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)

import signpost

print("Getting an edison client instance")
edison = signpost.EdisonApiClient()

print("Sending read_handle request")
block = 0
offset = 100
handle = block.to_bytes(4, 'big') + offset.to_bytes(4, 'big')
edison.send_read_handle(dest=signpost.ModuleAddress.Storage, handle=handle)

