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

class InstallCertificates:
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

        #get a list of all the files in the certs folder
        certList = os.listdir("./certs")
        
        print("\nConverting der format to pem format...")

        #If there are *.cer certs convert them to pem certs
        for file in certList:
            if(file[-3:] == "cer"):
                subprocess.call(['openssl','x509','-inform','der','-in','./certs/'+file,'-out','./certs/'+file[:-3]+'pem'])

        #now for all the pem certificates, write them to the ublox devices
        certList = os.listdir("./certs")
        for file in certList:
            if(file[-3:] == "pem"):
                #get the length of the file and the number of newlines
                length = subprocess.check_output(['wc','-c','./certs/'+file]).split()[0]
                
                #open the file
                cert = open('certs/'+file,'r');

                print("Writing " + file + " with length " + str(length))
                
                #send the command
                self.sp.write('AT+USECMNG=0,0,"'+file+'",'+str(length)+'\r')
                time.sleep(0.5)

                #send the contents of the file
                cert_str = cert.read()
                self.sp.write(cert_str)
                time.sleep(1)

            time.sleep(1)
        
        #clear the serial buffer
        self.sp.read(5000)

        #now print off all the files on the module
        self.sp.write('AT+USECMNG=3\r')
        certs = self.sp.read(5000)
        print("\nCertificates now on device:")
        print(certs)

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

    install_certificates = InstallCertificates(args)
    success = install_certificates.open(port=args.port)
    if not success:
        print('Could not open the serial port. Make sure the board is plugged in.')
        sys.exit(1)

    install_certificates.run()

if __name__ == '__main__':
    main()
