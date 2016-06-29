set terminal postscript enhanced eps color font "Helvetica,18" size 10in,5in

set output out

# Grayscale-safe but also pretty colors [mostly here for colors, change line settings willy nilly]
set style line 1 lt 1 ps 1.0 pt 5 lw 1 lc rgb "#fdae61"	# Orange
set style line 2 lt 1 ps 1.0 pt 7 lw 1 lc rgb "#d7191c"	# Dark Red
set style line 3 lt 1 ps 1.0 pt 9 lw 1 lc rgb "#abdda4"	# Green
set style line 4 lt 1 ps 0.3 pt 7 lw 1 lc rgb "#2b83ba"	# Blue
set style line 5 lt 1 ps 1.0 pt 2 lw 1 lc rgb "#FF1493"	# Pink
set style line 6 lt 1 ps 1.0 pt 3 lw 1 lc rgb "#6F0AAA"	# Purple

set border 3

set xtics nomirror rotate
set ytics nomirror

#set grid
set nokey

set datafile separator ","

set title tit

set ylabel "Estimated Max Power (W)" 
set xlabel "Time"

set datafile separator ","


set xdata time
set timefmt "%m/%d %H:%M:%S"

set xrange[start:end]

set label "Average = ".meanstring." W" at start,mean+0.2
set arrow 1 from start,mean to end,mean nohead

plot in using 1:4 with points linestyle 4
