#!/usr/bin/python
import serial.tools.list_ports
import subprocess
import sys
import os

ports = list(serial.tools.list_ports.comports())

#get a list of the port vendor IDs and Serial numbers
serial_dict = {}
for port in ports:
    if(port.vid == 1027):
        serial_dict[port.location[-1]] = port.serial_number;

if(len(serial_dict) != 6):
    print("There are not the right number of devices on this USB hub!");
    print("Expecting 6, found {}".format(len(serial_dict)));
    sys.exit(1)

for location in serial_dict:
    if(location == "7"):
        print("Configuring Module 0");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-Module0"')
    elif(location == "6"):
        print("Configuring Module 1");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-Module1"')
    elif(location == "4"):
        print("Configuring Controller");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-Controller"')
    elif(location == "3"):
        print("Configuring Storage Master");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-StorageMaster"')
    elif(location == "2"):
        print("Configuring Debug Radio");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-Radio"')
    elif(location == "1"):
        print("Configuring Debug Radio Debug");
        os.system('sudo ./ft232r_prog --old-serial-number ' + serial_dict[location] + ' --manufacturer Lab11 --product "Signpost-Debug-RadioDBG"')

print("Configured debug backplane successfully")
