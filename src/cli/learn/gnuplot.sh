#!/bin/bash

#open two terminal windows (gnuplot needs to run in foreground!!)
setsid gnome-terminal -- bash -c "gnuplot -e \"file='frame_ir.dat'; plottitle='Single Frame Mode'\" plot.gp" &
setsid gnome-terminal -- bash -c "gnuplot -e \"file='streaming_ir.dat'; plottitle='Streaming Mode'\" plot.gp" &
