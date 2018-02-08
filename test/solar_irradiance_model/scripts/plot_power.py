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
    avlist = []
    tavlist = []
    count = 0
    for row in reader:
        dt = row[0]
        time = pendulum.strptime(dt,"%m/%d/%Y %H:%M")
        if(time > start and time < end):

            index = (time - start).in_days()

            powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
            apowers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]

            try:
                avlist[index][0] += apowers[0]
                avlist[index][1] += apowers[1]
                avlist[index][2] += apowers[2]
                avlist[index][3] += apowers[3]
                count += 1
            except:
                if(len(avlist) > 0):
                    avlist[index-1][0] = avlist[index-1][0]/count
                    avlist[index-1][1] = avlist[index-1][1]/count
                    avlist[index-1][2] = avlist[index-1][2]/count
                    avlist[index-1][3] = avlist[index-1][3]/count

                avlist.append(apowers)
                tavlist.append(dt)
                count = 1

            plist.append(powers)
            tlist.append(dt)

    if(len(avlist) > 0):
        avlist[index][0] = avlist[index][0]/count
        avlist[index][1] = avlist[index][1]/count
        avlist[index][2] = avlist[index][2]/count
        avlist[index][3] = avlist[index][3]/count


    plt.plot(tlist, plist)
    plt.show()

    plt.plot(tavlist, avlist)
    plt.show()
