#!/usr/bin/python

import sys
import os
import csv
import numpy

out_folder = '../plots/';
in_folder = '../raw/';

for i in os.listdir(os.getcwd() + '/' + in_folder):
	#get the starting date
	info = i.split('-');
	start_month= info[0];
	start_day = info[1];

	start = start_month + '/' + start_day + ' 00:00:00';
	startsearch = start_month+'/'+start_day + ' 0:0';

	#get the ending date
	end_month = info[3];
	end_day = info[4];

	end = end_month + '/' + end_day + ' 23:59:59';
	endsearch = end_month + '/' + end_day + ' 23:59';

	#get other info
	angle = info[6];
	heading = info[7][:-4];

	#get the average for the start and the end dates
	sindex = 0;
	f = open(in_folder + i, 'r');
	for j, line in enumerate(f,1):
		if startsearch in line:
			sindex = j;

	eindex = 0;
	for k, line in enumerate(f,1):
		if endsearch in line:
			eindex = k;

	csv = numpy.genfromtxt(in_folder + i, delimiter=",", skip_header=1);
	if(eindex == 0):
		mean =  numpy.mean(csv[sindex:,3]);
	else:
		mean =  numpy.mean(csv[sindex:eindex,3]);

	#get the files as the will be passed
	inarg = "in='" + in_folder + i + "';";
	outarg = "out='" + out_folder + i[:-4] + ".eps';";
	startarg = "start='" + start + "';";
	endarg = "end='" + end + "';";
	meanarg = "mean=" + "{:.2f}".format(mean) + ";";
	meanarg2 = "meanstring='" + "{:.2f}".format(mean) + "';";
	titlearg = "tit='Solar Panel Test, " + heading + ", " + angle + ", 0.055m^2, 19% efficiency";

	print 'gnuplot -e "' + inarg+outarg+startarg+endarg+meanarg+meanarg2+titlearg + '" power_plotter.plt'
	os.system('gnuplot -e "' + inarg+outarg+startarg+endarg+meanarg+meanarg2+titlearg + '" power_plotter.plt');
