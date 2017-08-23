#!/usr/bin/env python


import argparse
import binascii
import os
import re
import struct
import sys
import time
import subprocess


import serial
import serial.tools.list_ports
import serial.tools.miniterm

class ProvisionRadio:
    def __init__ (self, args):
        self.debug = args.debug

    # Open the serial port to the chip/bootloader
    def open (self, port):

        # Check to see if the serial port was specified or we should find
        # one to use
        if port == None:
            print('No serial port specified. Discovering attached serial devices...')
            ports = list(serial.tools.list_ports.grep('FT232R'))
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
        self.sp.timeout= 1.0
        # Try to set initial conditions, but not all platforms support them.
        # https://github.com/pyserial/pyserial/issues/124#issuecomment-227235402
        self.sp.dtr = 0
        self.sp.rts = 0
        self.sp.open()

        return True


    def run (self):

        #clear the serial ports
        self.sp.write("AT\r")
        ret = self.sp.read(8)
        time.sleep(0.1)

        #turn off the echo
        self.sp.write("ATE0\r")
        ret = self.sp.read(10)
        time.sleep(0.1)

        #write the CGDCONT Command
        self.sp.write('AT+CGDCONT=1,"IP","neo.iot.net"\r')
        ret = self.sp.read(10)
        if(ret == "\r\nOK\r\n"):
            print("Finished Successfully");
        else:
            print("Did not get 'OK' Response");

        time.sleep(0.1)


################################################################################
## Setup and parse command line arguments
################################################################################

def main ():
    parser = argparse.ArgumentParser()

    # All commands need a serial port to talk to the board
    parser.add_argument('--port', '-p',
        help='The serial port to use')

    parser.add_argument('--debug',
        action='store_true',
        help='Print additional debugging information')

    args = parser.parse_args()

    provision_radio = ProvisionRadio(args)
    success = provision_radio.open(port=args.port)
    if not success:
        print('Could not open the serial port. Make sure the board is plugged in.')
        sys.exit(1)

    provision_radio.run()

if __name__ == '__main__':
    main()
