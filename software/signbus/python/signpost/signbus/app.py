#!/usr/bin/env python3

class AppLayer():
    def __init__(self, *, protocol_layer):
        self._protocol = protocol_layer
        self._lookup_key = lambda address: None

    def send(self, *, dest, frame_type, api_type, message_type, payload):
        buf = bytes()

        buf += frame_type.to_bytes(1, 'big')
        buf += api_type.to_bytes(1, 'big')
        buf += message_type.to_bytes(1, 'big')
        buf += bytes(payload)

        key = self._lookup_key(dest)

        self._protocol.send(dest, buf, key)

    def recv():
        raise NotImplementedError

