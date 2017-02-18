#!/usr/bin/env python3

import argparse
import binascii
import os
import struct
import sys
import time
import http.client

import serial
import serial.tools.list_ports
import serial.tools.miniterm

from _version import __version__


################################################################################
## Main Interface
################################################################################

class FakeRadio:
    def __init__ (self, args):
        self.debug = args.debug


    # Open the serial port to the chip/bootloader
    def open (self, port):

        # Check to see if the serial port was specified or we should find
        # one to use
        if port == None:
            print('No serial port specified. Discovering attached serial devices...')
            # Start by looking for one with "fake_radio" in the description
            ports = list(serial.tools.list_ports.grep('fake_radio'))
            if len(ports) > 0:
                # Use the first one
                print('Using "{}"'.format(ports[0]))
                port = ports[0][0]
            else:
                return False

        # Open the actual serial port
        self.sp = serial.Serial()
        self.sp.port = port
        self.sp.baudrate = 115200
        self.sp.parity=serial.PARITY_NONE
        self.sp.stopbits=1
        self.sp.xonxoff=0
        self.sp.rtscts=0
        self.sp.timeout=0.5
        # Try to set initial conditions, but not all platforms support them.
        # https://github.com/pyserial/pyserial/issues/124#issuecomment-227235402
        self.sp.dtr = 0
        self.sp.rts = 0
        self.sp.open()

        return True


    def run (self):
        while True:
            buf = self.sp.read(4096)

            if((len(buf)) == 0):
                continue

            if(chr(buf[0]) != "$"):
                #this is a debuggin message, ignore it
                continue

            if(len(buf) < 5):
                print("packet too short")
                continue;

            buf = buf[1:]
            url_len_struct = struct.unpack('<H', buf[0:2])
            url_len = url_len_struct[0]
            buf = buf[2:]
            url = buf[0:url_len].decode("utf-8")
            buf = buf[url_len:]
            num_headers = struct.unpack('<B', buf[0:1])[0]
            buf = buf[1:]
            headers = {}
            for i in range(0,num_headers):
                header_len = struct.unpack('<B',buf[0:1])[0]
                buf = buf[1:]
                header = buf[0:header_len].decode("utf-8")
                buf = buf[header_len:]
                value_len = struct.unpack('<B',buf[0:1])[0]
                buf = buf[1:]
                value = buf[0:value_len].decode("utf-8")
                buf = buf[value_len:]
                headers[header] = value


            body_len = struct.unpack('<H', buf[0:2])[0]
            buf = buf[2:]
            body = buf[:body_len]

            #now that we have parsed the buffer, post
            #split url into the first and second parts
            s_index = url.find("/")
            base = url[:s_index]
            end = url[s_index:]

            # is the base the gdp address?
            if(base == "gdp.lab11.eecs.umich.edu"):
                if(end == "/v1/create_log"):
                    pass
                else:
                    pass
            else:
                #this is a real http post. let's do it
                print("")
                print("#######################################################")
                print("Trying to post to {}".format(url))
                print("Post headers: {}".format(headers))
                print("Post body: {}".format(body))
                print("")
                try:
                    conn = http.client.HTTPConnection(base)
                    conn.request("POST",end,body,headers)
                    response = conn.getresponse()
                except:
                    print("Post failed, please check your destination URL")
                    print("")
                    print("")

                #we should send this back, but for now that's good
                print("Post Succeeded! See response below.")
                print("Status: {}, Reason: {}".format(response.status,response.reason))
                print("Body: {}".format(response.read().decode("utf-8")))
                print("")
                print("Sending response back to radio")
                print("#######################################################")
                print("")






################################################################################
## Setup and parse command line arguments
################################################################################

def main ():
    parser = argparse.ArgumentParser(add_help=False)

    # All commands need a serial port to talk to the board
    parser.add_argument('--port', '-p',
        help='The serial port to use')

    parser.add_argument('--debug',
        action='store_true',
        help='Print additional debugging information')

    parser.add_argument('--version',
        action='version',
        version=__version__,
        help='Tockloader version')

    args = parser.parse_args()

    fake_radio = FakeRadio(args)
    success = fake_radio.open(port=args.port)
    if not success:
        print('Could not open the serial port. Make sure the board is plugged in.')
        sys.exit(1)

    print("Starting fake-radio server. Listening for commands....")
    print("")
    fake_radio.run()



if __name__ == '__main__':
    main()
