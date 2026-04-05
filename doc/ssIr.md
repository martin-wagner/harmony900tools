# Harmony 900 SsIr.bin

## Overview

SsIr.bin stores the commands learned in Harmony Software using _raw mode_. The file contains raw pulse/pause data and carrier clock frequency for each single command using this mode.

## General Structure

The file contains header, offset table and data array of variable length:

```
hhhhh 1 ss oo oo oo ddddddd ddd ddddddddddd
    │ │  │  │  │          │   └── data[1]     
    │ │  │  │  │          └── data[0]     
    │ │  │  │  └── start offset data[1]
    │ │  │  └── start offset data[0]
    │ │  └── Command count (=data array count)
    │ └── ???
    └── Header
```

All data except the header is encoded as 16 bit little endian words ( v16 = array8[i] | (array8[i+1] << 8) ).

## Header

Header always contains 

```
01 01 05 00 00 01 ...
```

The function of the header is unknown. The first 5 bytes are the file header and are excluded from offset calculation. The sixth bytes is always 1.

## Command count

This field contains the command count and is needed for the calculation of the offset table and array size.

```
... 02 00 ...
```

equals to two raw commands.

## Offset table

This field contains the start offsets of the individual commands. Each command has one offset. The offset excludes the header (5 bytes).

```
... 07 00 0d 03 ...
     │     │      
     │     └── start offset command 1 781+5 = raw[786]
     └── start command 0 at 7+5 = raw[12]
```

Each command uses all available space, e.g. command 0 uses raw[12] ... raw[785].

## Payload

The payload contains the IR carrier clock frequency (ToDo verify this!) and mark/pause data.
Mark = Transmitter is on  
Pause = Transmitter is off.  

Mark is detected by checking the MSB.


```
6784 0000 0026 8035 06b7 836c 0386 86e6 0384 836e 0382 8370 0382 836f 06fb
   │    │    │    │    └── space (1719µs)
   │    │    │    └── MSB set -> Mark (53µs)
   │    │    └── Sample count (38)
   │    └── ??? always 0
   └── 1 / f_carrier 26,5µs / 37.8kHz
```

It is unclear if multiple _space_ commands can follow each other to prolong the pause time to an inter-frame gap.
