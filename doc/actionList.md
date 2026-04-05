# Harmony 900 ActionList.xml

## Overview

the ActionList.xml contains info about Device Actions. Why this nees a separate file is unknown.

use e.g.
```
xmllint --format ActionList.xml > ActionList-pretty.xml' 
```
to make it human-readable. Don't forget to name it back before writing a modified file to your remote!

## Hash

The xml contains the IrProto.bin crc32 in two places. After changing IrProto.bin you need to update the _ProtocolCacheHash_ and _Hash_ members accordingly.