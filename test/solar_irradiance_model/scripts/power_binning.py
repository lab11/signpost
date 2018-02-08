#!/usr/bin/env python3

import sys
import os
import csv
import numpy as np
import pendulum
import datetime
import matplotlib.pyplot as plt

if(len(sys.argv) < 2):
    print("You must pass a input data file and date ranges into this script")
    sys.exit(1)

input_folder = sys.argv[1]
infiles = []
for dirpath,_,filenames in os.walk(input_folder):
    for f in filenames:
        infiles.append(os.path.abspath(os.path.join(dirpath, f)))

#30,35,40,45,50
power_list = [[],[],[],[],[]]
for f in infiles:
    with open(f, 'r') as inp:
        reader = csv.read(inp)
        ll = next(reader)[1]
        lat = ll.split(',')[0]
        next(reader)

        for row in reader:
            if lat >= 27.5 and lat < 32.5:
                power_list[0].append([int(row[1]),int(row[2]),int(row[3]),int(row[4])])
            elif lat >= 32.5 and lat <= 37.5:
                power_list[1].append([int(row[1]),int(row[2]),int(row[3]),int(row[4])])
            elif lat >= 37.5 and lat <= 42.5:
                power_list[2].append([int(row[1]),int(row[2]),int(row[3]),int(row[4])])
            elif lat >= 42.5 and lat <= 47.5:
                power_list[3].append([int(row[1]),int(row[2]),int(row[3]),int(row[4])])
            elif lat >= 47.5 and lat <= 52.5:
                power_list[4].append([int(row[1]),int(row[2]),int(row[3]),int(row[4])])
            else:
                pass


print(len(power_list[0]))
print(len(power_list[1]))
print(len(power_list[2]))
print(len(power_list[3]))
print(len(power_list[4]))
