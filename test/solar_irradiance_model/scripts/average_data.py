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
output_fname = sys.argv[2]
days = sys.argv[3]

with open(output_fname, 'w') as out:
    with open(input_fname, 'r') as inp:
        reader = csv.reader(inp)

        #skip the first header line
        head = next(reader)
        out.write(head[0] + ',' + head[1] + '\n')
        head = next(reader)
        out.write(head[0] + ',' + head[1] + ',' + head[2] + ',' + head[3] + ',' + head[4] + '\n')

        avlist = [0,0,0,0]
        tavlist = []
        count = 0
        first = True
        for row in reader:
            dt = row[0]
            time = pendulum.strptime(dt,"%m/%d/%Y %H:%M")
            if(first):
                last_time = time
                first = False

            if((time - last_time).in_days() >= int(days)):
                out.write(last_time.strftime("%m/%d/%Y %H:%M") + ',' + str(avlist[0]/count) + ',' + str(avlist[1]/count) + ',' + str(avlist[2]/count) + ',' + str(avlist[3]/count) + '\n')
                last_time = time
                count = 1
                powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
                avlist[0] = powers[0]
                avlist[1] = powers[1]
                avlist[2] = powers[2]
                avlist[3] = powers[3]
            else:
                powers = [float(row[1]),float(row[2]),float(row[3]),float(row[4])]
                avlist[0] += powers[0]
                avlist[1] += powers[1]
                avlist[2] += powers[2]
                avlist[3] += powers[3]
                count += 1
        out.write(last_time.strftime("%m/%d/%Y %H:%M") + ',' + str(avlist[0]/count) + ',' + str(avlist[1]/count) + ',' + str(avlist[2]/count) + ',' + str(avlist[3]/count) + '\n')
