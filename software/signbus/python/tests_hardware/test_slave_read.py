#!/usr/bin/env python3

import logging
logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)

import signpost

print("Getting an edison client instance")
edison = signpost.EdisonApiClient()

print("Performing slave read")
edison.read_from_slave(dest=signpost.ModuleAddress.Storage, count=100)


