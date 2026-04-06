# Harmony 900 UserConfiguration.xml

## Overview

the UserConfiguration.xml contains your user config (most of the devices, all of your activities). It is stored as condensed xml. 

use e.g.
```
xmllint --format UserConfiguration.xml > UserConfiguration-pretty.xml' 
```
to make it human-readable. Don't forget to name it back before writing a modified file to your remote!

## Hash

The xml contains the IrProto.bin crc32 in two places. After changing IrProto.bin you need to update the _ProtocolCacheHash_ and _Hash_ members accordingly.

## Commands

The remote can reference protocols in irProto.bin and commands in SsIr.bin. Index begins at [0]!

### ssIr

A command in ssIr is referenced by inserting the command number like in the following example:

```
 <Command>
   <Name>Off</Name>A
   <Data>
     <Protocol>-1</Protocol>
     <Code>0xFFFF0200</Code>
   </Data>
 </Command>
```
where ssIr is selected as protocol _-1_ and the command is indexed by "0x0002".

### irProto

A protocol in irProto is referenced by inserting the protocol number like in the following example:

```
 <Command>
   <Name>PowerOff</Name>
   <Data>
     <Protocol>7</Protocol>
     <Code>0x0700F401030001009C6000</Code>
   </Data>
 </Command>
```
where _7_ is the protocol index and _0x07..._ is the parameter for command encoding.

```
Offset   Bytes       Value    Meaning
[0:1]    07 00       7        Protocol index → IrProto.bin
[2:3]    F4 01       500      Carrier period in system clock ticks (LE16)
                              500 ticks @ 18 MHz = 27.778 µs = 36.000 kHz 
[4:5]    03 00       3, 0     Timing section / repeat parameters (?)
[6:7]    01 00       1, 0     Timing section / repeat parameters (?)
[8]      9C          0x9C     Bit stream
[9]      60          0x60     Bit stream
[n]      xx          0xx      Bit stream
[10/n+1] 00          0        Trailing zero / terminator
```
The bit stream is MSB aligned, so in conjunction with the bit count in irProto you get the valid bit stream. More bits are coded by adding another byte.
todo verify -- ai generated...

The UserConfiguration.xml also contains the info about if and where to place toggle bits. todo -- is this also available somwhere in the bit stream / IrProto.bin?