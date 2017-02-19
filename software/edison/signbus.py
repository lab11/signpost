#!/usr/bin/env python3

import hashlib
import os

try:
    import cryptography
except:
    print("pip install --user cryptography")
    raise

try:
    import periphery
except:
    print("pip install --user python-periphery")
    raise


class I2C():
    MAX_MESSAGE_SIZE = 255

    FLAG_FRAGMENT_SHIFT = 0

    def __init__(self, device="/dev/i2c-6", source_address=0x40):
        self._i2c = periphery.I2C(device)
        self._address = source_address

        self._sequence_number = 0

    # Net layer
    def send(self, dest, data):
        # make a local copy
        data = bytes(data)

        # buffer for common bits
        buf = bytes()

        # array of I2C transactions
        msgs = []

        # version + flags
        # n.b. fragment flag handled later
        version = 0x1
        flags = 0x0
        buf += ((version << 4) | flags).to_bytes(1, 'big')

        # source
        buf += self._source_address.to_bytes(1, 'big')

        # seq_num
        self._sequence_number += 1
        buf += self._sequence_number.to_bytes(2, 'big')

        # length
        buf += len(data).to_bytes(2, 'big')

        overhead = buf.len()
        goodput_len = I2C.MAX_MESSAGE_SIZE - overhead

        while len(data) > goodput_len:
            to_send = bytes(buf)

            # set fragment flag
            to_send[0] |= (1 << I2C.FLAG_FRAGMENT_SHIFT)

            # move a chunk of data to send buffer
            to_send += data[:goodput_len]
            data = data[goodput_len:]
            assert len(to_send) == I2C.MAX_MESSAGE_SIZE

            # add this to send list
            msgs.append(periphery.I2C.Message(to_send))

        # send the last chunk
        to_send = bytes(buf)
        to_send += data[:goodput_len]
        data = data[goodput_len:]
        msgs.append(periphery.I2C.Message(to_send))

        # issue the I2C transactions
        i2c.trasfer(dest, msgs)

    def recv():
        raise NotImplementedError

    # XXX Copied from ⤵  ; change I2C -> periphery.I2C ; untested
    # https://github.com/lab11/erpc/blob/signpost_erpc/erpc_python/erpc/i2cmessage.py
    def read(self, dest, count):
        maxLen = 247
        numPackets = int(math.ceil(float(count)/maxLen))

        toReceive = count
        dataToReturn = bytearray(count)
        while toReceive > 0:
            if toReceive > maxLen:
                rec = bytearray(255)
                msg = [periphery.I2C.Message(rec,read=True)]
                self._i2c.transfer(dest,msg)
                dataToReturn[count-toReceive:count-toReceive+maxLen] = msg[0].data[8:]
                toReceive = toReceive - maxLen
            else:
                rec = bytearray(8+toReceive)
                msg = [periphery.I2C.Message(rec,read=True)]
                self._i2c.transfer(dest,msg)
                dataToReturn[count-toReceive:] = msg[0].data[8:]
                toReceive = 0

        return dataToReturn


# https://cryptography.io/en/latest/hazmat/primitives/symmetric-encryption/
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

class ProtocolLayer():
    def __init__(self, net):
        self._backend = default_backend()
        self._net = net

    def send(self, dest, data, key=None):
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

        self._net.send(dest, to_send)

    def recv():
        raise NotImplementedError


class AppLayer():
    def __init__(self, protocol):
        self._protocol = protocol
        self._lookup_key = lambda address: None

    def send(self, dest, frame_type, api_type, message_type, payload):
        buf = bytes()

        buf += frame_type.to_bytes(1, 'big')
        buf += api_type.to_bytes(1, 'big')
        buf += message_type.to_bytes(1, 'big')
        buf += bytes(payload)

        key = self._lookup_key(dest)

        self._protocol.send(dest, buf, key)

    def recv():
        raise NotImplementedError
