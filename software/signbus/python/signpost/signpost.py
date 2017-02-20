#!/usr/bin/env python3

import logging
log = logging.getLogger(__name__)

import enum

from . import signbus

class ModuleAddress(enum.IntEnum):
    Controller = 0x20,
    Storage = 0x21,
    Radio = 0x22,

class FrameType(enum.IntEnum):
    Notification = 0
    Command = 1
    Response = 2
    Error = 3

class ApiType(enum.IntEnum):
    Initialization = 1
    Storage = 2
    Networking = 3
    Processing = 4
    Energy = 5
    TimeLocation = 6
    Edison = 7

class EdisonApiMessageType(enum.IntEnum):
    ReadHandle = 0
    ReadRPC = 1

class EdisonApiClient():
    DEFAULT_EDISON_MODULE_ADDRESS = 0x40

    def __init__(self, *,
            signbus_instance=None):
        '''For use on an edison to talk to a stoarge master

        **Note:** The signbus object needs to own the I2C bus. It is important
        that there is only one live signbus on a system at a time. If you need
        to support multiple clients, you must pass in an existing signbus.
        '''
        if signbus_instance is None:
            # Create an instance
            signbus_instance = signbus.Signbus(source_address=EdisonApiClient.DEFAULT_EDISON_MODULE_ADDRESS)
        self._signbus = signbus_instance

    def send_read_handle(self, *,
            dest,
            handle):
        '''handle should be an array of 8 bytes'''
        self._signbus.send(
                dest=dest,
                frame_type=FrameType.Notification, # XXX: notifcation b/c no reply
                api_type=ApiType.Edison,
                message_type=EdisonApiMessageType.ReadHandle,
                payload=handle,
                )

    def send_read_rpc(self):
        raise NotImplementedError

