#!/usr/bin/env python3

import argparse
import binascii
import os
import struct
import sys
import time

import serial
import serial.tools.list_ports
import serial.tools.miniterm

from ._version import __version__


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
		ret = self.sp.read(1024)
		print(ret)


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

	fake_radio.run()



if __name__ == '__main__':
	main()
