Debug Backplane Commissioning
=============================

These scripts name the serial ports of the debug backplane.

The individual scripts are suitable for programming one FTDI chip, and
only work properly if one FTDI chip is currently attached to your computer.

The commissional_all.py script will reprogram multiple FTDI chips attached to
a hub such is the case with the new version of the debug backplane. It
finds all devices and names the ftdi chips based on their location on 
the debug backplane hub.

Both scripts utilize [ft232r_prog](http://rtr.ca/ft232r/) which uses libftdi-dev
to rename the devices.  

You probably want to install libftdi-dev and run these scripts 
on a linux computer due to mac problems with libftdi.
