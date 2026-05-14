# Harmony 900 UserConfiguration.xml

## Overview

The UserConfiguration.xm is a single XML document with `<Root>` as top-level element. It stores approx. 90% of the info the Harmony software needs to describe one user's remote setup: meta info, the remote controller, all controlled devices, all activities. It contains part of the IR protocols.  
Other relevant files are:
- actionList.xml
- irProto.bin
- ssIr.bin

---

The file is stored as condensed xml. use e.g.
```
xmllint --format UserConfiguration.xml > UserConfiguration-pretty.xml' 
```
to make it human-readable. Don't forget to name it back before writing a modified file to your remote!

**Top-Level Structure**

```
<Root>
  <Properties>        # file meta: version, hash, last update timestamp
  <User>              # user ID, preferences, name
  <Controller>        # the remote itself
  <Device> × N        # one entry per controlled device
  <Activity> × M      # one entry per activity (watch mode)
  <Protocols>         # IR protocol definitions used across all devices
```

For some of the values, it's unknown wether they are actually used by the remote or if they are just a hint for the online harmony software. This is especially true for the `<Properties>`.

## `<Properties>`

Simple key/value bag for file-level metadata.

| Property | Example value |
|---|---|
| `version` | `1.0` |
| `ProtocolCacheHash` | `0x960E98B9` |
| `LastUpdated` | `20260406 143318` |

## `<User>`

Holds language info and the user's display name. Other fields seem to be optional.

| Language | LocaleId | 
|---|---|
| English-US | enu |
| German | deu |

## `<Controller>`

Describes the physical remote control itself – manufacturer, model, type. In this config it is labeled as a "Harmony 1000-ish" by Logitech.

Be aware that the Harmony 1000 uses a completly different setup type!

## `<Device>`

One `<Device>` element per piece of AV equipment. Examples:

| your label | Type | Manufacturer / Model |
|---|---|---|
| TV | Television | LG ... |
| Video recorder | Vcr | Samsung SV... |
| 4k bd | DvdCd | Panasonic DP-... |
| PVR VU | Pvr | Vu+ ... |
| AV Receiver-Room 1 | Receiver | Onkyo TX-NR... |
| and so on... |  |  |

For more details see [device.md](device.md)

## `<Activity>`

One `<Activity>` element per watch/use mode. Examples:

| Label | Type |
|---|---|
| PowerOff | PowerOff |
| Watch TV | VirtualTelevisionN |
| Listen to Radio | VirtualRadioSimple |
| Watch movie | VirtualDvd |
| and so on... |  |

PowerOff is a dummy activity and is always present.

For more details see [activity.md](activity.md)

## `<Protocols>`

Only actual info seems to be `<ToggleBitX>`, which is redundant to irProto.bin.

## Hash

The xml contains the IrProto.bin crc32 in two places. After changing IrProto.bin you need to update the _ProtocolCacheHash_ and _Hash_ members accordingly.
