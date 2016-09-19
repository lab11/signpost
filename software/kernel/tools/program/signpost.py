#!/usr/bin/env python2

import time
from sys import argv
import os

BUILD_DIR = 'tock/build/'
JLINK_SCRIPT = 'platform/controller/apps.jlink'

argv.pop(0)

img = ""

for filename in argv:
    f = open(filename)
    img = img + f.read()
    f.close()

print("Writing %d files totaling %d bytes" % (len(argv), len(img)))

# Padding zeroes at the end
img = img + '\x00\x00\x00\x00\x00\x00\x00\x00'

# Write to output file so we can flash them
with open(BUILD_DIR+'apps.bin', 'w') as app_bin:
    app_bin.write(img)

# Flash apps with JLink
then = time.time()
ret = os.system('JLinkExe -device ATSAM4LC8C -if swd -speed 1200 -AutoConnect 1 ' + JLINK_SCRIPT)
if ret != 0:
    print("Error loading apps")
    sys.exit(1)

now = time.time()
print("Wrote %d bytes in %.3f seconds" %(len(img), now-then))

