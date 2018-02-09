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

BINS = 5000
RMAX = 5

#30,35,40,45,50
counts30 = np.zeros((BINS,4,1))
counts35 = np.zeros((BINS,4,1))
counts40 = np.zeros((BINS,4,1))
counts45 = np.zeros((BINS,4,1))
counts50 = np.zeros((BINS,4,1))
seattlecounts = np.zeros((BINS,4,1))
seattlemask = 0
sdcounts = np.zeros((BINS,4,1))
sdmask = 0
cmask = np.zeros((5))
for f in infiles:
    with open(f, 'r') as inp:
        reader = csv.reader(inp)
        line = next(reader)
        lat = line[0].split(' ')[-1]
        lon = line[1]
        lat = float(lat)
        lon = float(lon)
        next(reader)

        location_list = []
        for row in reader:
            location_list.append([float(row[1]),float(row[2]),float(row[3]),float(row[4])])

        loc = np.array(location_list)
        mask = loc < 0
        loc[mask] = 0

        #there is a bug in some of our files??
        loccount = np.zeros((BINS,4,1))
        if(location_list[0][0] > 30):
            continue

        for i in range(0,4):
            c, e = np.histogram(loc[:,i],bins=BINS,range=(0,RMAX),density=True)
            c = c/(BINS/RMAX)
            loccount[:,i,0] = c


        # the seattle area
        if lat > 47 and lat < 49 and lon < -122 and lon > -124:
            if(seattlemask == 0):
                seattlecounts = loccount
                seattlemask = 1
            else:
                seattlecounts = np.append(seattlecounts,loccount,axis=2)

        if lat > 32 and lat < 34 and lon < -115 and lon > -117:
            if(sdmask == 0):
                sdcounts = loccount
                sdmask = 1
            else:
                sdcounts = np.append(sdcounts,loccount,axis=2)

        if lat >= 27.5 and lat < 32.5:
            if(cmask[0] == 0):
                counts30 = loccount
                cmask[0] = 1
            else:
                counts30 = np.append(counts30,loccount,axis=2)
        elif lat >= 32.5 and lat <= 37.5:
            if(cmask[1] == 0):
                counts35 = loccount
                cmask[1] = 1
            else:
                counts35 = np.append(counts35,loccount,axis=2)
        elif lat >= 37.5 and lat <= 42.5:
            if(cmask[2] == 0):
                counts40 = loccount
                cmask[2] = 1
            else:
                counts40 = np.append(counts40,loccount,axis=2)
        elif lat >= 42.5 and lat <= 47.5:
            if(cmask[3] == 0):
                counts45 = loccount
                cmask[3] = 1
            else:
                counts45 = np.append(counts45,loccount,axis=2)
        elif lat >= 47.5 and lat <= 52.5:
            if(cmask[4] == 0):
                counts50 = loccount
                cmask[4] = 1
            else:
                counts50 = np.append(counts50,loccount,axis=2)
        else:
            continue


#now we should be able to collapse these and put them into a counts array
counts = np.zeros((BINS,4,5))
counts[:,:,0] = np.sum(counts30,axis=2)/counts30.shape[2]
counts[:,:,1] = np.sum(counts35,axis=2)/counts35.shape[2]
counts[:,:,2] = np.sum(counts40,axis=2)/counts40.shape[2]
counts[:,:,3] = np.sum(counts45,axis=2)/counts45.shape[2]
counts[:,:,4] = np.sum(counts50,axis=2)/counts50.shape[2]
seattlecounts = np.sum(seattlecounts,axis=2)/seattlecounts.shape[2]
sdcounts = np.sum(sdcounts,axis=2)/sdcounts.shape[2]


#now flip the histogram, then do a cumsum
sums = np.cumsum(counts, axis=0)
seattlesums = np.cumsum(seattlecounts, axis=0)
sdsums = np.cumsum(sdcounts, axis=0)

icounts = np.flip(counts,axis=0)
iseattlecounts = np.flip(seattlecounts,axis=0)
isdcounts = np.flip(sdcounts,axis=0)

isums = np.cumsum(icounts, axis=0)
iseattlesums = np.cumsum(iseattlecounts, axis=0)
isdsums = np.cumsum(isdcounts, axis=0)


isums = np.flip(isums, axis=0)
iseattlesums = np.flip(iseattlesums, axis=0)
isdsums = np.flip(isdsums, axis=0)


x = np.arange(0,5,1/(BINS/RMAX))

#plt.figure(1)
#plt.plot(x,sums[:,0,:])
#plt.plot(x,seattlesums[:,0])
#plt.plot(x,sdsums[:,0])
#
plt.figure(2)
plt.plot(x,isums[:,0,:])
plt.plot(x,iseattlesums[:,0])
plt.plot(x,isdsums[:,0])

plt.figure(6)
plt.plot(x,isums[:,1,:])
plt.plot(x,iseattlesums[:,1])
plt.plot(x,isdsums[:,1])
#plt.figure(3)
#plt.plot(x,sums[:,2,:])

plt.figure(4)
plt.plot(x,isums[:,2,:])
plt.plot(x,iseattlesums[:,2])
plt.plot(x,isdsums[:,2])

#plt.figure(5)
#plt.plot(x,sums[:,1,:])


#plt.figure(7)
#plt.plot(x,sums[:,3,:])

plt.figure(8)
plt.plot(x,isums[:,3,:])
plt.plot(x,iseattlesums[:,3])
plt.plot(x,isdsums[:,3])

plt.show()



