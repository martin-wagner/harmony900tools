# Harmony 900 IrProto.bin

## Overview

IrProto.bin stores the protocols used in UserConfiguration.xml to serialise the data given as binary values. 

## General Structure

The file contains a size/crc header, inside this is another header with offset table and data arrays of variable length. This matches [SsIr.bin](ssIr.md).

### File header

The file contains header, file size and crc (both excluding the header):

```
b9 98 0e 96 f8 01 00 00 dd dd dd ...
          │     │     │  └── data 
          │     │     └── ??
          │     └── size of data (504 bytes)
          └── CRC32 over data


```

## Data structure

Data contains header, offset table and data array of variable length:

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

Data uses little endian encoding (16/32 bit words).

## Data Header

Header always contains 

```
01 01 05 00 00 01 ...
```

The function of the header is unknown. The first 5 bytes are the file header and are excluded from offset calculation. The sixth bytes is always 1.

## Protocol count

This field contains the protocol count and is needed for the calculation of the offset table and array size.

```
... 02 00 ...
```

equals to two available protocol descriptions.

## Protocol offset table

This field contains the start offsets of the individual protocols. Each protocol has one offset. The offset excludes both of the headers (5 + 8 bytes).

```
... 07 00 0d 03 ...
     │     │      
     │     └── start offset command 1 781+13 = raw[794]
     └── start command 0 at 7+13 = raw[20]
```

Each protocol uses all available space, e.g. command 0 uses raw[20] ... raw[794]. 

## Protocols

Each protocol consists of protocol-wide variables and one to n timing sections. The user parameters are set inside the (UserConfiguration.xml)](userConfiguration.md) file.

## Protocol params

Each protocol again consists of a header with an offset table. This works as before, all offsets are relative to the same start position.

```
... 01 81 6c 00 00 32 03 98 00 b5 00 dd dd dd dd ...
     │     │     │  │  │     │     │      
     │     │     │  │  │     │     └── start offset timing section 1 181+13 = raw[194]
     │     │     │  │  │     └── start offset timing section 0 at 152+13 = raw[165]
     │     │     │  │  └── Timing sections count 3
     │     │     │  └── ?? always 50
     │     │     └── ?? always 0
     │     └── 1 / f_carrier 27.8µs / 36kHz
     └── start byte (always 1)
```

## Timing section params

Each Timing section consists of parameters, yet another offset table and correspoinding timing values.


```
... 30 00 ff ff ff ff ff ff 02 02 13 01 05 01 0a 01 dd dd dd ...
        │     │           │  │  │     │     │     │
        │     │           │  │  │     │     │     └── start offset EoF timing section 1 266+13 = raw[279]
        │     │           │  │  │     │     └── start offset SoF timing section 1 261+13 = raw[274]
        │     │           │  │  │     └── start offset Payload timing section 275+13 = raw[288]
        │     │           │  │  └── Ctrl 1
        │     │           │  └── Ctrl 0
        │     │           └── time between Sof of this frame and SoF of the next frame in µs. -1 = pause coded in EoF data field
        │     └── Mask
        └── ir protocol data bit count 48 (for this section, excluding start/stop)
```

Mask: In Conjunction with the xml data bitstream, this seems to mask out bits. 0xff -- use as-is. Use case not fully understood.

Ctrl0:  
Frame Type
- 2: This timing section contains parameters for a data frame.
- 0: This timing section contains parameters for a repeat frame (only SoF/EoF used, no data)  

Ctrl1:  
Payload Data coding for 0/1 data bitstream from xml
- 0: Data field is empty / not used (repeat frame)
- 1: Data field contains one pair of timing values (usecase unknown. you can't code two binary states with this. Wii seems to use this, maybe only power-toggle?)
- 2: Data field contains two pairs of timing values (first pair = binary "0", second pair binary "1" todo verify order is not 1/0).

Offset: 0x0000 = NULL, field not used in this section.

### Timing section SoF/EoF

SoF and EoF both use the same format. This section contains a minimum of one mark value. We decode an EoF with included inter-frame gap as example:

```
... 04 a9 81 ff 7f 18 52 19 52 ...
     │     │     │     │     │     
     │     │     │     │     └── Timing value 5219 -> Pause, 21017µs
     │     │     │     └── Timing value 5218 -> Pause, 21016µs
     │     │     └── Timing value 7fff -> Pause, 32767µs
     │     └── Timing value 81a9 -> Mark, 425µs
     └── timing value counter 4
```

Mark = Transmitter is on  
Pause = Transmitter is off.  

Mark is detected by checking the MSB.

### Timing section payload

The size of the payload is given in Ctrl1 as timer pair count. 2 means two 2 byte pairs.

```
... a9 81 c2 01 a9 81 28 05 ...
        │     │     │     │     
        │     │     │     └── Timing value 0528 -> Pause, 1320µs
        │     │     └── Timing value 81a9 -> Mark, 425µs
        │     └── Timing value 01c2 -> Pause, 450µs
        └── Timing value 81a9 -> Mark, 425µs
     
```

Mark = Transmitter is on  
Pause = Transmitter is off.  

Mark is detected by checking the MSB.



