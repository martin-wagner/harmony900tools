# External parameters: file and plottitle
set terminal qt persist
set title plottitle
set xlabel 'Time (microseconds)'
set ylabel 'Amplitude (1=ON, 0=OFF)'
set style data steps
set yrange [-0.5:1.5]
set grid

plot file using 1:2 with steps lw 2 lc rgb 'blue' notitle

pause -1
