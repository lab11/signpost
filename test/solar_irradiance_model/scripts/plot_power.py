#!/usr/bin/env python3

import sys
import os
import csv
import numpy as np
import pendulum
import datetime
import matplotlib.pyplot as plt

if(len(sys.argv) < 4):
    print("You must pass a input data file and date ranges into this script")
    sys.exit(1)

input_fname = sys.argv[1]
start_date = sys.argv[2]
end_date = sys.argv[3]

start = pendulum.strptime(start_date,"%m/%d/%Y %H:%M")
end = pendulum.strptime(end_date,"%m/%d/%Y %H:%M")

with open(input_fname, 'r') as inp:
    reader = csv.reader(inp)

    #skip the first header line
    head = next(reader)
    head = next(reader)

    tlist = []
    plist = []
    for row in reader:
        dt = row[0]
        time = pendulum.strptime(dt,"%m/%d/%Y %H:%M")
        if(time > start and time < end):
            powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
            plist.append(powers)
            tlist.append(dt)


    plt.plot(tlist, plist)
    plt.show()
