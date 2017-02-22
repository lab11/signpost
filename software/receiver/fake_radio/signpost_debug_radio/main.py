#!/usr/bin/env python2

from __future__ import print_function

import argparse
import binascii
import os
import re
import struct
import sys
import time
import httplib

import serial
import serial.tools.list_ports
import serial.tools.miniterm
try:
    import gdp
except:
    pass

try:
    import colorama
    print_red = lambda x: print(colorama.Fore.RED + x + colorama.Style.RESET_ALL)
except:
    print_red = lambda x: print(x)

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
        self.sp.timeout= 1.0
        # Try to set initial conditions, but not all platforms support them.
        # https://github.com/pyserial/pyserial/issues/124#issuecomment-227235402
        self.sp.dtr = 0
        self.sp.rts = 0
        self.sp.open()

        return True


    def run (self):
        while True:
            #read one byte at a time in a loop
            by = self.sp.read(1)

            if(len(by) == 0):
                continue

            #if the byte is one of the escape characters read it in
            byte = None
            try:
                byte = by.decode("utf-8")
                #sys.stdout.write(byte)
            except:
                #it won't be valid if this fails
                continue

            buf = None
            if(byte == "#"):
                #see if the next character is a reset
                byte = self.sp.read(1);
                if(byte == "r"):
                    print("Fake-Radio Reset. Ready to receive radio Commands!")
                elif(byte == "w"):
                    #waiting on you to return data
                    #is this the state you should be in right now??
                    print("waiting on response");
                    pass
                elif(byte == "p"):
                    #waiting on you to return data
                    #is this the state you should be in right now??
                    print("Kernel Panic - dumping buffer");
                    #use a bug number just cause
                    buf = self.sp.read(16000);
                    print(buf.decode("utf-8"))
                    pass
                else:
                    sys.stdout.write("#" + byte)

                continue
            elif(byte == "$"):
                #this is an actual message
                #read two more bytes to get the length
                num_bytes = struct.unpack('<H', self.sp.read(2))[0]

                #read in length number of bytes
                buf = self.sp.read(num_bytes)

                #did we get the number of bytes or timeout?
                if(len(buf) < num_bytes):
                    #we have a long enough timeout this shouldn't happen
                    #disregard this message
                    print("Received buffer shorted than expected. Discarding")
                    continue
            else:
                sys.stdout.write(byte)
                continue


            #we have a valid buffer, we should parse it
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
            body = bytearray()
            body.extend(buf[:body_len])

            #now that we have parsed the buffer, post
            #split url into the first and second parts
            s_index = url.find("/")
            base = url[:s_index]
            end = url[s_index:]

            # is the base the gdp address?
            if(base == "gdp.lab11.eecs.umich.edu"):
                    stat = 0
                    reason = ""
                    print("")
                    print("#######################################################")
                    print("Trying to post to GDP")
                    index1 = 1+end[1:].find("/")
                    index2 = index1 + 1 + end[index1+1:].find("/")
                    index3 = index2 + 1 + end[index2+1:].find("/")
                    #version
                    try:
                        version = end[index1+1:index2]
                        log_name = end[index2+1:index3]
                        function = end[index3+1:]
                    except:
                        print("There was an error, aborting")
                        print("Do you have GDP installed?")
                        print("#######################################################")
                        print("")
                        continue

                    if(function == "append" or function == "Append"):
                            print("Attempting to append to log name {}".format(log_name))
                            #try to create the log. Don't know how to do this in python
                            #so instead call the shell
                            ret = os.system("gcl-create -C lab11-signpost@umich.edu -k none " + log_name)
                            if((ret >> 8) == 0):
                                print("Successfully created log")
                                stat = 201
                                reason = "OK - Log Created"
                            elif((ret >> 8) == 73):
                                print("Log already exists")
                                stat = 200
                                reason = "OK"
                            else:
                                print("An unkown gdp error(code {}) occurred).".format(str((ret >> 8))))
                                stat = 500
                                reason = "Server Error"

                            try:
                                gcl_name = gdp.GDP_NAME(log_name)
                                gcl_handle = gdp.GDP_GCL(gcl_name,gdp.GDP_MODE_AO)
                                gcl_handle.append({"signpost-data": body})
                                print("Append success")
                            except:
                                print("There was an error, aborting")
                                stat = 500
                                reason = "Server Error"
                    else:
                        print("Does not support that function")
                        stat = 503
                        reason = "Service Unkown"

                    #form the response here based on some of the stats above
                    send_buf = bytearray()
                    send_buf.extend(struct.pack('<H',stat))
                    send_buf.extend(struct.pack('<H',len(reason)))
                    send_buf.extend(reason)
                    send_buf.extend(struct.pack('<B',2))

                    send_buf.extend(struct.pack('<B',len("content-type")))
                    send_buf.extend("content-type")
                    send_buf.extend(struct.pack('<B',len("application/octet-stream")))
                    send_buf.extend("application/octet-stream")

                    send_buf.extend(struct.pack('<B',len("content-length")))
                    send_buf.extend("content-length")
                    send_buf.extend(struct.pack('<B',len("1")))
                    send_buf.extend("1")
                    send_buf.extend(struct.pack('<H',1))
                    send_buf.extend(struct.pack('<B',0x00))
                    self.sp.write(send_buf);
                    print("#######################################################")
                    print("")

            else:
                #this is a real http post. let's do it
                print("")
                print("#######################################################")
                print("Trying to post to {}".format(url))
                print("Post headers: {}".format(headers))
                if re.match('^[\x0a-\x7F]+$', body):
                    # all bytes in body are printable characters
                    print("Post body: {}".format(body))
                else:
                    print("Post body: <binary data, length {}>".format(len(body)))
                    print('  ' + ' '.join(map(lambda x: str.format('{:02x}', x), body)))
                print("")
                try:
                    conn = httplib.HTTPConnection(base)
                    conn.request("POST",end,body,headers)
                    response = conn.getresponse()
                except:
                    print("Post failed, please check your destination URL")
                    print("#######################################################")
                    print("")
                    continue


                #we should send this back, but for now that's good
                print("Post Succeeded! See response below.")
                print("Status: {}, Reason: {}".format(response.status,response.reason))
                body = response.read();
                print("Body: {}".format(body))
                print("")
                #now format the response and send it back to the radio
                send_buf = bytearray()
                send_buf.extend(struct.pack('<H',response.status))
                send_buf.extend(struct.pack('<H',len(response.reason)))
                send_buf.extend(response.reason)
                send_buf.extend(struct.pack('<B',len(response.getheaders())))
                for header in response.getheaders():
                    send_buf.extend(struct.pack('<B',len(header[0])))
                    send_buf.extend(header[0])
                    send_buf.extend(struct.pack('<B',len(header[1])))
                    send_buf.extend(header[1])
                send_buf.extend(struct.pack('<H',len(body)))
                send_buf.extend(body)
                self.sp.write(send_buf);
                print("Sending response back to radio")
                print("#######################################################")
                print("")




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


    try:
        import gdp
    except:
        print("")
        print_red("Failed to import gdp. There will be no gdp support for this session")
        print("")
        print("If you are on a debian based machine, to fix download and install:")
        print("    https://github.com/lab11/signpost/blob/master/software/receiver/fake_radio/gdp-packages/python-gdp_0.7.2_all.deb")
        print("    https://github.com/lab11/signpost/blob/master/software/receiver/fake_radio/gdp-packages/gdp-client_0.7.2_all.deb")
        print("    sudo dpkg -i python-gdp_0.7.2_all.deb gdp-client_0.7.2-1_amd64.deb")
        print("    sudo apt-get -f install")
        print("    sudo dpkg -i python-gdp_0.7.2_all.deb gdp-client_0.7.2-1_amd64.deb")
        print("We don't know how to get it to work on mac. Please contact the GDP team for support.")

    print("")
    print("Starting fake-radio server. Listening for commands....")
    print("")
    fake_radio.run()



if __name__ == '__main__':
    main()
