#!/usr/bin/env python3

import logging
log = logging.getLogger(__name__)

import hashlib
import os

try:
    import cryptography
except:
    print("pip install --user cryptography")
    raise

# https://cryptography.io/en/latest/hazmat/primitives/symmetric-encryption/
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

class ProtocolLayer():
    def __init__(self, *, network_layer):
        self._backend = default_backend()
        self._net = network_layer

    def send(self, *, dest, data, key=None):
        # make a copy of data
        data = bytes(data)

        # handle optionally requested encryption
        if key is not None:
            # AES 256. Library selects using key length.
            assert len(key) == 32

            iv = os.urandom(16)
            cipher = Cipher(algorithms.AES(key), modes.CTR(iv), backend=self._backend)
            encryptor = cipher.encryptor()
            encrypted = encryptor.update(data) + encryptor.finalize()

            #>>> decryptor = cipher.decryptor()
            #>>> decryptor.update(ct) + decryptor.finalize()
        else:
            encrypted = data

        # Compute SHA256 digest
        digest = hashlib.sha256(encrypted).digest()

        # prepare mesage and send down
        to_send = encrypted + digest

        self._net.send(dest=dest, data=to_send)

    def recv():
        raise NotImplementedError
