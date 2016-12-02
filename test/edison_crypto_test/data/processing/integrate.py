#!/usr/bin/env python3

import argparse
import csv

parser = argparse.ArgumentParser()
parser.add_argument('file', help='csv file')
parser.add_argument('start', type=float, help='start x value')
parser.add_argument('end', type=float, help='end x value')
args = parser.parse_args()

s = 0
a = []
with open(args.file) as csvf:
    reader = csv.reader(csvf, delimiter=',')
    for i in range(16):
        next(reader, None)
    for row in reader:
        x, y, m = row
        x = float(x)
        y = float(y)
        if x < args.start or x > args.end: continue
        a.append([x,y])
for i in range(1,len(a)):
    s += (a[i][0] - a[i-1][0]) * (a[i][1] + a[i-1][1])/2
print(s*3.3)
