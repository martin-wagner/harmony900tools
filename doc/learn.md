# Harmony 900 IR learning protocol

## Overview

Harmony 900 has a tcp server listening on 169.254.1.2:3074 after initialising the USB/network driver in Windows. No user/pass, plain binary data. This server implements the IR learning protocol. It supports two modes:
- single frame mode. In this mode, the remote waits for IR reception and returns a single frame + inter-frame silence after reception is completed.
- streaming mode. In this mode, the remote continuosly receives and returns data. In case no transmitter is active, silence is returned.

Both commands return raw timing values in microseconds.

Be aware that on protocol violations the server will crash/become unresponsive. Reboot your remote.

## Protocol

Not everything is fully understood. The script "/usr/local/share/lua/5.1/ethanol/SysService.lua" -- _ircap_ on the file system contains the base protocol in binary form.
The protocol consists of start, data exchange and close commands. For data exchange single and stream are available.

__Start capture__

Command 0xA1
```
Send: 20 a1 80 01 01 00
Recv: 20 a1 01 00 — ACK
```

No further information about the function of the bytes, except for the command.

__Read single frame__

Command 0xA2
```
Send: 20 a2 80 00
Recv: 20 a2 01 05 01 01 02 xx xx 02 xx xx 02 xx xx 02 xx xx  — ACK OK + Payload
             │        │     └── Payload
             │        └── EoF (no)
             └── status (ok)

Recv: 20 a2 01 05 01 00 02 xx xx 02 xx xx 02 xx xx 02 xx xx  — ACK OK, EoF + Payload
             │        │     └── Payload
             │        └── EoF (yes)
             └── status (ok)

Recv: 20 a2 02 00  — ACK Timeout
             └── status (timeout)
```
Status:  
0x01 OK  
0x02 Timeout  
other ?? maybe other errors

EoF:  
0x01 more data available  
0x00 all data is read

Usage: Poll using command 0xa2. When IR is received, the command will return the first data block. Then read using command 0xa2 until either state != OK or EoF is set. The command will always return four two byte payload words.

To get the byte stream, drop the "0x02" and just append all payload bytes read into a byte array.

Bytes not marked have unknown functionality, but seem to be constant.

__Read stream__

Command 0xA3
```
Send: 20 a3 80 00
Recv: 20 a3 01 02 70 xx xx xx xx ... xx xx 01 30  — ACK + Payload
                      └── Payload           └── EoF
```
EoF:  
0x0130 end of frame

Usage: Poll using command 0xa3. When the buffer is filled, the command will return. When no IR is active, the buffer will be filled with "silence" (takes longer). The command will always return 96 two byte payload words.
You can have this command running for as long as you want.

To get the byte stream, just append all payload bytes read into a byte array.

Bytes not marked have unknown functionallity, but seem to be constant.

__End capture__

Command 0xA4
```
Send: 20 a4 80 00
Recv: 20 a4 01 00 — ACK
```

No further information about the function of the bytes, except for the command.

## Payloads

Payload is always 16 bit big endian words ( v16 = array8[i+1] | (array8[i] << 8) ). Each Value represents a time in microseconds.  
Mark = Transmitter is on  
Pause = Transmitter is off.  
ToDo -- where is the carrier frequency??

### Single Frame

Full transfer:
```
opening connection
-------------
11:44:38:737 -> capture started: 20 a1 80 01 01 00
11:44:38:758 <- confirmation frame: 20 a1 01 00
press remote
poll single frame
-------------
11:44:38:758 -> poll sections: 20 a2 80 00
11:44:39:582 <- section frame: 20 a2 01 05 01 01 02 00 23 02 03 41 02 00 1e 02 06 f3
11:44:39:582 -> poll sections: 20 a2 80 00
11:44:39:590 <- section frame: 20 a2 01 05 01 01 02 06 ea 02 0a 6b 02 03 71 02 0a 6a
11:44:39:590 -> poll sections: 20 a2 80 00
11:44:39:594 <- section frame: 20 a2 01 05 01 01 02 03 73 02 06 f3 02 03 71 02 06 f1
11:44:39:594 -> poll sections: 20 a2 80 00
11:44:39:597 <- section frame: 20 a2 01 05 01 01 02 06 ec 02 0a 64 02 03 7a 02 06 fa
11:44:39:597 -> poll sections: 20 a2 80 00
11:44:39:601 <- section frame: 20 a2 01 05 01 01 02 03 72 02 0a 6b 02 03 72 02 06 ea
11:44:39:601 -> poll sections: 20 a2 80 00
11:44:39:653 <- section frame: 20 a2 01 05 01 01 02 06 f3 02 0a 74 02 03 71 02 80 00
11:44:39:653 -> poll sections: 20 a2 80 00
11:44:39:696 <- section frame: 20 a2 01 05 01 00 02 00 00 02 80 00 02 00 00 02 65 ae
<> idle detected -> end capture
closing connection
-------------
11:44:44:445 -> close connection: 20 a4 80 00
11:44:44:449 <- confirmation frame: 20 a4 01 00
command received!
```

Appending all payload bytes and converting to big-endian words leads to the following timings:
```
00035 00833 00030 01779 01770 02667 00881 02666 00883 01779 00881 01777 01772 02660 00890 01786 00882 02667 00882 01770 01779 02676 00881 32768 00000 32768 00000 26030
```

Decoding:
```
00035 00833 00030 01779 01770 02667 00881 02666 00883 01779
    │     │     │     │     │     │     │     │     └ rinse and repeat
    │     │     │     │     │     │     │     └── Mark + Pause
    │     │     │     │     │     │     └── Mark
    │     │     │     │     │     └── Mark + Pause
    │     │     │     │     └── Mark
    │     │     │     └── Mark + Pause
    │     │     └── ???
    │     └── Mark
    └── ???

```

Mark = 833us  
Pause = 1779us - 833us = 946us

Mark = 1770us  
Pause = 2667us - 1770us = 897us

...

Mark = 881us  
Pause = 32768 - 881us = 31905us  
Mark = 0us  
Pause = 32768us  
Mark = 0us  
Pause = 26030us  
-> Pause = 31905us + 32768us + 26030us = 90721us
```
RC5 Manchester
┐ ┌─┐   ┌─┐ ┌───┐ ┌─┐ ┌─┐   ┌─┐ ┌─┐ ┌───┐ ┌─┐   ┌─┐ ┌───────── 90ms inter-frame gap
└─┘ └───┘ └─┘   └─┘ └─┘ └───┘ └─┘ └─┘   └─┘ └───┘ └─┘
1   1   0   0   1   1   1   0   0   0   1   1   0   0
```

The content/use of the two "???" is unknown.

### Stream

Full transfer:
```
opening connection
-------------
11:44:38:737 -> capture started: 20 a1 80 01 01 00
11:44:38:758 <- confirmation frame: 20 a1 01 00
poll stream data
-------------
11:44:39:697 -> poll data: 20 a3 80 00
11:44:40:046 <- data frame: 20 a3 01 02 70 03 72 06 f3 ... 06 f3 01 30
poll stream data
-------------
11:44:40:046 -> poll data: 20 a3 80 00
11:44:40:488 <- data frame: 20 a3 01 02 70 06 ea 0a 62 ... 06 eb 01 30
poll stream data
-------------
11:44:40:488 -> poll data: 20 a3 80 00
11:44:40:943 <- data frame: 20 a3 01 02 70 06 f2 0a 72 ... 06 f2 01 30
poll stream data
-------------
11:44:40:943 -> poll data: 20 a3 80 00
11:44:41:295 <- data frame: 20 a3 01 02 70 06 ea 0a 6b ... 0a 72 01 30
poll stream data
-------------
11:44:41:295 -> poll data: 20 a3 80 00
11:44:42:873 <- data frame: 20 a3 01 02 70 03 72 80 00 ... 80 00 01 30
poll stream data
-------------
11:44:42:873 -> poll data: 20 a3 80 00
11:44:44:444 <- data frame: 20 a3 01 02 70 00 00 80 00 ... 80 00 01 30
closing connection
-------------
11:44:44:445 -> close connection: 20 a4 80 00
11:44:44:449 <- confirmation frame: 20 a4 01 00
```

Appending all payload bytes leads to a similar structure as the single frame format:
```
00882 01779 01770 02666 00882 02667 00882 01779 00881 01777 01771 02660 00889 01786 00881 02667 00882 01770 01779 02676 00881 32768 00000 32768 00000 26030 00881 01779 ...
```

The two words with unknown use are not present. Decoding works the same, except that the frame can repeat multiple times, depening on how long recording was active and how long the button on the source remote was pressed.
Be aware that for some formats, the first frame is different to the repeats, so the stream must be running before pressing the button.  
When no IR is active, "0000 8000" is placed to indicate no mark, 32ms pause

Decoding:
```
00882 01779 01770 02666 00882 02667 00882 01779
    │     │     │     │     │     │     └ rinse and repeat
    │     │     │     │     │     └── Mark + Pause
    │     │     │     │     └── Mark
    │     │     │     └── Mark + Pause
    │     │     └── Mark
    │     └── Mark + Pause
    └── Mark

```


