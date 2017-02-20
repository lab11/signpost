#!/usr/bin/env python3

import logging
log = logging.getLogger(__name__)

try:
    import periphery
except:
    print("pip install --user python-periphery")
    raise

class NetworkLayer():
    MAX_MESSAGE_SIZE = 255

    FLAG_FRAGMENT_SHIFT = 0

    def __init__(self, *, source_address, device="/dev/i2c-6"):
        self._i2c = periphery.I2C(device)
        self._source_address = source_address

        self._sequence_number = 0

    # Net layer
    def send(self, *, dest, data):
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

        overhead = len(buf)
        goodput_len = NetworkLayer.MAX_MESSAGE_SIZE - overhead

        while len(data) > goodput_len:
            to_send = bytes(buf)

            # set fragment flag
            to_send[0] |= (1 << NetworkLayer.FLAG_FRAGMENT_SHIFT)

            # move a chunk of data to send buffer
            to_send += data[:goodput_len]
            data = data[goodput_len:]
            assert len(to_send) == NetworkLayer.MAX_MESSAGE_SIZE

            # add this to send list
            msgs.append(periphery.I2C.Message(to_send))

        # send the last chunk
        to_send = bytes(buf)
        to_send += data[:goodput_len]
        data = data[goodput_len:]
        msgs.append(periphery.I2C.Message(to_send))

        # issue the I2C transactions
        log.debug("dest {}")
        for msg in msgs:
            log.debug("\t msg {}".format(msg))
        self._i2c.transfer(dest, msgs)

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
