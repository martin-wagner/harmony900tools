set terminal qt 1 persist
set xlabel 'Time (microseconds)'
set ylabel 'Amplitude (1=ON, 0=OFF)'
set style data steps
set yrange [-0.5:1.5]
set grid
set title 'Single Frame Mode'
plot 'frame_ir.dat' using 1:2 with steps lw 2 lc rgb 'blue' notitle

set terminal qt 2 persist
set xlabel 'Time (microseconds)'
set ylabel 'Amplitude (1=ON, 0=OFF)'
set style data steps
set yrange [-0.5:1.5]
set grid
set title 'Streaming Mode'
plot 'streaming_ir.dat' using 1:2 with steps lw 2 lc rgb 'red' notitle
pause -1
