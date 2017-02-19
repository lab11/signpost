#!/usr/bin/env python3

from enum import Enum

import signbus

class ModuleAddress(Enum):
    ModuleAddressController = 0x20,
    ModuleAddressStorage = 0x21,
    ModuleAddressRadio = 0x22,

class FrameType(Enum):
    NotificationFrame = 0
    CommandFrame = 1
    ResponseFrame = 2
    ErrorFrame = 3

class ApiType(Enum):
    InitializationApiType = 1
    StorageApiType = 2
    NetworkingApiType = 3
    ProcessingApiType = 4
    EnergyApiType = 5
    TimeLocationApiType = 6
    EdisonApiType = 7

class EdisonApiMessageType(Enum):
    EdisonReadHandleMessage = 0
    EdisonReadRPCMessage = 1

class EdisonApiClient():
    DEFAULT_EDISON_MODULE_ADDRESS = 0x40

    def __init__(self, *,
            signbus=None,
            ):
        '''For use on an edison to talk to a stoarge master

        **Note:** The signbus object needs to own the I2C bus. It is important
        that there is only one live signbus on a system at a time. If you need
        to support multiple clients, you must pass in an existing signbus.
        '''
        if signbus is None:
            # Create an instance
            signbus = signbus.Signbus(source_address=EdisonApiClient.DEFAULT_EDISON_MODULE_ADDRESS)
        self._signbus = signbus

    def send_read_handle(self, *,
            dest,
            handle,
            ):
        '''handle should be an array of 8 bytes'''
        self._signbus.send(
                dest=dest,
                frame_type=NotificationFrame, # XXX: notifcation b/c no reply
                api_type=EdisonApiType,
                message_type=EdisonReadHandleMessage,
                payload=handle,
                )

    def send_read_rpc(self, *,
            ):
        raise NotImplementedError

