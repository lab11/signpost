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
        reader = csv.reader(inp)
        lat = next(reader)[0].split(' ')[-1]
        lat = float(lat)
        next(reader)

        for row in reader:
            if lat >= 27.5 and lat < 32.5:
                power_list[0].append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])
            elif lat >= 32.5 and lat <= 37.5:
                power_list[1].append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])
            elif lat >= 37.5 and lat <= 42.5:
                power_list[2].append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])
            elif lat >= 42.5 and lat <= 47.5:
                power_list[3].append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])
            elif lat >= 47.5 and lat <= 52.5:
                power_list[4].append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])
            else:
                pass


#okay now we should convert these to numpy arrays
p30 = np.array(power_list[0])
p35 = np.array(power_list[1])
p40 = np.array(power_list[2])
p45 = np.array(power_list[3])
p50 = np.array(power_list[4])

nplist = [p30, p35, p40, p45, p50]

#sort each of the columns in the array for each cardinal direction
nplist[0] = np.sort(nplist[0],axis=0)
nplist[1] = np.sort(nplist[1],axis=0)
nplist[2] = np.sort(nplist[2],axis=0)
nplist[3] = np.sort(nplist[3],axis=0)
nplist[4] = np.sort(nplist[4],axis=0)

#if any entries are less than 0 - assign zero
mask = nplist[0] < 0
nplist[0][mask] = 0

mask = nplist[1] < 0
nplist[1][mask] = 0

mask = nplist[2] < 0
nplist[2][mask] = 0

mask = nplist[3] < 0
nplist[3][mask] = 0

mask = nplist[4] < 0
nplist[4][mask] = 0

#get counts for each lat

BINS = 5000
RMAX = 5
counts = np.zeros((BINS,4,5))
#create a counts array so that we can generate the cdf
for i in range(0,5):
    for j in range(0,4):
        c, e = np.histogram(nplist[i][:,j],bins=BINS,range=(0,RMAX),density=True)
        counts[:,j,i] = c/(BINS/RMAX)

#now flip the histogram, then do a cumsum
sums = np.cumsum(counts, axis=0)
icounts = np.flip(counts,axis=0)
isums = np.cumsum(icounts, axis=0)
isums = np.flip(isums, axis=0)
x = np.arange(0,5,.1)

#plt.figure(1)
#plt.plot(x,counts[:,0,:])
#print(counts[:,0,:])
#
#plt.figure(2)
#plt.plot(x,isums[:,0,:])
#
#plt.figure(3)
#plt.plot(x,counts[:,2,:])
#
##plt.figure(4)
##plt.plot(x,icounts[:,2,:],'.')
#
#plt.figure(5)
#plt.plot(x,counts[:,1,:])
#
##plt.figure(6)
##plt.plot(x,icounts[:,1,:],'.')
#
#plt.figure(7)
#plt.plot(x,counts[:,3,:])
#
##plt.figure(8)
##plt.plot(x,icounts[:,3,:],'.')
#plt.show()
#
#
#
#
#















