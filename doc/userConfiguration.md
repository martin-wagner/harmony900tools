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

 Offset |    Bytes     |  Value  |  Meaning
 ---|---|---|---
[0:1]  |    07 00     |  7      |  Protocol index → IrProto.bin
[2:3]  |    F4 01     |  500    |  Carrier period in system clock ticks (LE16)
       |              |         |  500 ticks @ 18 MHz = 27.778 µs = 36.000 kHz 
[4]    |    03        |  3      |  Repeats
[5]    |    00        |  0      |  Special repeat frame (true = 1, false = 0)
       |              |         |  Either repeats or repeat frame should be used.
[6]    |    01        |  S1     |  Control
       |              |         |  - 0 = Flat data coding
       |              |         |  - 1 = Single section data coding
       |              |         |  - >1 = Multi section data coding
[7...e-1] |           |  payload | Depends on Control
[e]    |    00        |  0      |  Trailing zero / terminator

The header is always 6 bytes + terminator byte long. The payload size depends on the control.

Values in _Control_ are assumptions. Could as well be a direct reference to a protocol/protocol family
- 0 = NEC (or generic + repeat)
- 1 = Generic (toggle info in <CodeSequence index>)
- 2 = ???
- 3 = Philips RC-6
...

** Control _Flat_ **

`0x0000F401010100 -->20DF18E70101<-- 00`

Offset    | Bytes      | Value   | Meaning
---|---|---|---
[7...e-3] | 20DF18E7   | bits    | Payload. The bit stream is MSB aligned, so in conjunction with the bit count in irProto you get the valid bit stream. More bits are coded by adding another byte.
[7...e-2] | 01         | 1       | unknown (always 1)
[7...e-1] | 01         | 1       | unknown (always 1)


** Control _Single Section_ **

`0x0100F401030001 -->005F5F11EE<-- 00`

Offset    | Bytes       | Value   | Meaning
---|---|---|---
[7]       | 00          | 0       | unknown (always 0)
[8...e-1] | 5F5F11EE    | bits    | Payload. The bit stream is MSB aligned, so in conjunction with the bit count in irProto you get the valid bit stream. More bits are coded by adding another byte.

** Control _Multi Section_ **

Each section is two bytes long and maps directly to the section in IrProto.bin.

`0x0200F401030003 -->0070010002B992<-- 00`

Offset    | Bytes       | Value   | Meaning
---|---|---|---
[7]       | 00          | 0       | unknown (always 0)
[8:9]     | 70 01       | bits    | Payload section 1. The bit stream is MSB aligned, so in conjunction with the bit count and mask in irProto you get the valid bit stream.
[10:11]   | 00 02       | bits    |  Payload section 2. ...
[12:13]   | B9 92       | bits    |  Payload section 3. ...

I have one sample of this, for a device using the Philips RC-6 code. 
- Section 0 codes 4 bit start in the first byte (first nibble 0x7, second nibble ??). Use of second byte (0x02) unknown.
- Section 1 codes 1 bit, but in RC-6 this is the toggle. Maybe second byte == 1 -> toggle?
- Section 2 is data.
RC-6 and Philips RCMM seem to be the only protocols to have different encodings for true/false within a single frame.





