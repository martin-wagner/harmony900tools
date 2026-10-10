# Harmony 900 UserConfiguration.xml -- Device List

## Overview

Each `<Device>` represents one piece of AV equipment the remote can control. Devices have no dependencies on each other and no dependencies on activities. They are referenced by their `<Id>` from activity configuration.

---

## Top-level Tree

```
<Device>
├── <Id>                   Unique numeric ID (referenced throughout the file)
├── <Type>                 Device class string
├── <Manufacturer>         Manufacturer name
├── <Model>                Model string
├── <Presentation>         UI representation / Button assignment
│   ├── <Label>            Display name assigned by the user
│   ├── <ControlGroup name="Misc">        Soft-button assignments [OPTIONAL]
│   │   └── <Button> × N
│   │       ├── <Label>    Button label (may be empty → icon-only)
│   │       ├── <Position> Zero-based display order
│   │       └── <ActionId> "<DeviceId>_<CommandName>_Hold"
│   └── <ControlGroup name="HardButtons"> Physical remote button mappings
│       └── <Button name=...> × N  name of hard button this command will be assigned to.
│           ├── <Label>    Button label, always empty
│           └── <ActionId> "<DeviceId>_<CommandName>_Hold"
├── <Properties>           Capability and behaviour flags
│   └── <Property name="...">  value </Property> × N
├── <States>               State machine for this device  [OPTIONAL]
│   └── <State> × N
│       ├── <Id>           State name (Power, Input, Screen, ...)
│       ├── <Value> × N    Enumerated legal values for this state
│       ├── <Delay>        Wait time in ms after a state change  [OPTIONAL]
│       ├── <DiscreteActions>  Direct value→command mappings  [OPTIONAL]
│       │   ├── <SetAction> × N   Force state to a specific value
│       │   │   ├── <Action>      The IR/state operation to perform
│       │   │   └── <Name>        Target value name this action applies to
│       │   └── <ChangeAction> × N  On transition to a value
│       │       ├── <Action>
│       │       └── <Name>
│       └── <RelativeActions>  Cycle-based transitions  [OPTIONAL]
│           ├── <NextAction>   Advance to next value (cycle)
│           │   ├── <Action>
│           │   └── <Name>    (optional)
│           ├── <PrevAction>   Go to previous value  [OPTIONAL]
│           │   └── <Action>
│           └── <ResetAction>  Reset a linked state to a fixed value  [OPTIONAL]
│               └── <Action>
├── <Numeric>              Channel/number entry configuration  [OPTIONAL]
│   ├── <FixedDigits>      0 = variable length; N = exact digit count required.
│   ├── <Finish>           Action sent after number entry is complete  [OPTIONAL]
│   │   └── <Action>       Typically SendCommand(OK/Enter)
│   ├── <FirstDigit>       Digit entry slots for position 1
│   │   └── <Digit> × 10   One per digit 0–9, each contains an <Action>
│   ├── <MiddleDigit>      Digit entry for middle positions  [OPTIONAL]
│   │   └── <Digit> × 10
│   └── <LastDigit>        Digit entry for last position  [OPTIONAL]
│       └── <Digit> × 10
└── <Commands>             IR command library
    ├── <Properties>       Timing defaults for all commands in this device
    │   ├── <Property name="PressPreSilence">   ms before press IR burst
    │   ├── <Property name="PressInterKey">     ms between press repeat bursts
    │   ├── <Property name="HoldPreSilence">    ms before hold IR burst
    │   └── <Property name="HoldInterKey">      ms between hold repeat bursts
    └── <Command> × N      One per learnable/defined button
        ├── <Name>         Button name (matches ActionId references)
        └── <Data>
            ├── <Protocol> Index into root <Protocols> list or escape for raw command
            └── <Code>     Hex-encoded IR code blob or index for raw command
```

---

## Field Details

### `<Id>`

Globally unique numeric ID assigned by the Logitech server when the device is registered. Used as a foreign key throughout the file: in activity roles, power lists, action parameters, and button `<ActionId>` prefixes.

Format: integer string, e.g. `91738261`.

---

### `<Type>`

Device class. Controls which UI templates and role types the Harmony software offers.

The following values have been seen:

| Value | Description |
|---|---|
|Amplifier | Simple amplifier (no input switching) |
|AudioVideoSwitch||
|Cd||
|ClimateControl||
|Computer||
|DvdCd||
|DvdCdGame||
|DvdCdRadio||
|GameConsole||
|Light||
|MediaCenterPC||
|Projector||
|Pvr||
|Receiver | AV receiver / pre-amp with input switching |
|SetTopBox|
|Television|
|Vcr|

This list is most likely incomplete.

---

### `<Presentation>`

#### `<Label>`

Free-text display name. User-defined, not constrained.

#### `<ControlGroup name="Misc">`

Soft button assignments — which soft button triggers which device command. Those are all that don't have a hard button assigned. Maps logical button names to ActionIds.

May be absent on devices where all buttons where mapped to hard buttons.

#### `<ControlGroup name="HardButtons">`

Physical button assignments — which physical remote key triggers which device command. Always present. Button labels are always empty.

Harmony 900 has the following hard buttons:

- Blue
- ChannelDown
- ChannelUp
- DirectionDown
- DirectionLeft
- DirectionRight
- DirectionUp
- DownArrow
- Exit
- FastForward
- Green
- Guide
- Info
- Menu
- Number0
- Number1
- Number2
- Number3
- Number4
- Number5
- Number6
- Number7
- Number8
- Number9
- NumberEnter
- NumberPlus
- Pause
- Play
- PrevChannel
- Record
- Red
- Rewind
- Select
- SkipBack
- SkipForward
- Stop
- UpArrow
- VolumeDown
- VolumeMute
- VolumeUp
- Yellow

You can't map the activities and help button.

#### `<Button>`

| Field | Type | Notes |
|---|---|---|
| `<Label>` | string | Display label; may be empty for icon-only buttons |
| `<Position>` | integer ≥ 0 | Display/layout order |
| `<ActionId>` | string | Format: `<DeviceId>_<CommandName>_Hold` |

The `_Hold` suffix is always present.

---

### `<Properties>`

All properties are `<Property name="...">value</Property>` elements. Boolean values are `true`/`false` (lowercase) or occasionally `True`/`False` (mixed case – inconsistency in the data).

| Property                          | Type    | Values seen     | Meaning                                                                                                                                                         |
| --------------------------------- | ------- | --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `AlwaysOn`                        | bool    | `true`, `false` | Hint that the device is typically left on. Does **not** guarantee it's always in the On list of every activity — the activity's Power section is authoritative. |
| `AudioSwitch`                     | bool    |                 |                                                                                                            |
| `AutoPower`                       | bool    | `false`         | TV-specific. Whether the TV auto-powers via CEC or similar.                                                                                                     |
| `Dimmer`                          | bool    | `false`, `true` | Whether the light device supports dimming.                                                                                                                      |
| `HasBands`                        | bool    | `true`          | Device supports AM/FM band switching (Receiver, MediaCenterPC).                                                                                                 |
| `HasPresets`                      | bool    | `true`          | Device supports named channel/station presets.                                                                                                                  |
| `IsDisplayDevice`                 | bool    | `true`          | Marks the device as a display/screen. Drives which device gets the DISPLAY role.                                                                                |
| `IsNewDevice`                     | bool    | `true`          | Set when a device was recently added. Likely a UI hint.                                                                                                         |
| `ManualPower`                     | bool    | `true`, `false` | `false` = remote auto-controls power via state machine. `true` = device must be powered manually; no `Power` state will be present.                             |
| `MenuOnDevice`                    | bool    | `false`         | Menu/UI is rendered on the device itself (not the remote).                                                                                                      |
| `NumDiscs`                        | integer | `1`, `5`        | Number of disc slots.                                                                                                                                           |
| `NumLights`                       | integer | `1`, `12`       | Number of individually controllable light channels.                                                                                                             |
| `OnScreenGuide`                   | bool    | `false`         |                                                                                                                    |
| `PvrType`                         | string  | `Generic`       | PVR/DVR class.                                                                                                                                                  |
| `RecordMedia Fixed Disc`          | bool    | `true`          | Device has a fixed internal disc for recording.                                                                                                                 |
| `RecordMedia Removable Videotape` | bool    | `true`          | Device uses removable tape (VCR-specific).                                                                                                                      |
| `RevertInput`                     | bool    | `true`          | TV-specific. Possibly reverts input on power cycle.                                                                                                             |
| `Scart`                           | bool    | `true`          | TV has a SCART connector.                                                                                                                                       |
| `TunerInput`                      | string  | `Tuner`         | Names the input that connects to an antenna/tuner signal.                                                                                                       |
| `VideoSwitch`                     | bool    | `true`          |                                                                                                          |


This list is most likely incomplete.

**Key relationship:** `ManualPower=false` always correlates with the device having a `Power` State. `ManualPower=true` always means no `Power` State. This is a hard structural dependency.

---

### `<States>`

Optional. Absent for devices the remote cannot auto-control (ManualPower=true) and for the Amplifier.

Contains one or more `<State>` elements, each tracking one aspect of device state.

#### State IDs

| State Id | Devices using it | Values | Delay (ms) |
|---|---|---|---|
| `Power` | Television, Vcr, DvdCd, Pvr, Receiver, Light | `Off`, `On` | 1500–40000 (boot time) |
| `Input` | Television, Pvr, Receiver, MediaCenterPC | device-specific list | 300–1000 |
| `Screen` | Television (LG only) | input name aliases | none |
| `TVInput` | Television | `TV`, `DTV`, `Radio` | none |
| `AntennaOutput` | Vcr | `Antenna`, `Vcr` | none |

This list is most likely incomplete.

The `<Delay>` reflects the device's time needed to process the command, e.g. boot time – how long the remote waits after sending a power-on command before issuing the next action. This is critical for reliable activities.

#### `<DiscreteActions>` vs `<RelativeActions>`

**DiscreteActions** – used when the device supports direct selection of a value (e.g. discrete input select commands). Contains:

- `<SetAction>`: fired when software forces a state to a specific value. Each has a `<Name>` (the target value) and an `<Action>`.
- `<ChangeAction>`: fired on any transition to a value. Semantically used for Power On (since Off→On is the relevant transition, and there's no "force to On" without sending PowerOn).

**RelativeActions** – used when the device only has a cycle/toggle command. Contains:

- `<NextAction>`: advance to the next value in the enumeration.
- `<PrevAction>`: go back to the previous value.
- `<ResetAction>`: reset a *different* state to a fixed value

#### Action structure

```
<Action>
  <Target>  "Device" (only value seen)
  <Operation>
    <Name>      operation type
    <Parameter> device ID (always first)
    <Parameter> command name / state name / value / delay ms
    <Parameter> "Press" (when SendCommand)
```

Operation types found in state actions:

| Operation | Meaning |
|---|---|
| `ForceValue` | Forcibly set a state value without triggering its actions (seen once: VCR ResetAction) |
| `SendCommand` | Send an IR command to the device |
| `SendDelay` | insert a pause |
| `SetValue` | Update another state on the same device (chain reaction) |

\todo warum ist das notwendig?
#### Interesting: TV has a two-level input state chain

The LG TV has both `Input` and `Screen` states. When `SetValue(TV, Input, 'HDMI 2')` is called:
1. The `Input` state's `SetAction` for `HDMI 2` fires.
2. That action is `SetValue(TV, Screen, 'HDMI2')`.
3. The `Screen` state's `ChangeAction` for `HDMI2` fires.
4. That action is `SendCommand(TV, InputHdmi2, Press)`.

This is a two-level indirection. The `Input` state holds the user-visible name (`HDMI 2`), while `Screen` holds the internal alias (`HDMI2`) that matches the command name.

---

### `<Numeric>`

Optional. Present only on devices that support channel/number entry (TVs, PVRs, VCRs, ...).

| Field | Type | Meaning |
|---|---|---|
| `<FixedDigits>` | integer | `0` = user confirms after any number of digits; `N` = exactly N digits must be entered before auto-confirm |
| `<Finish>` | Action | Command sent after number entry (usually `OK` or `Enter`). Optional – absent if FixedDigits handles completion. |
| `<FirstDigit>` | 10 × Digit | One Action per digit 0–9 for the first position |
| `<MiddleDigit>` | 10 × Digit | Actions for all middle positions (optional) |
| `<LastDigit>` | 10 × Digit | Actions for the last position (optional) |

Each `<Digit>` contains exactly one `<Action>` which sends the corresponding digit command (e.g. `SendCommand(deviceId, '3', Press)`).

The `<FirstDigit>` group is always present if `<Numeric>` exists. `<MiddleDigit>` and `<LastDigit>` are optional – their absence implies the same digit commands apply in all positions.

---

### `<Commands>`

The IR command library for this device.

**First entry** is always an `<Properties>` timing-defaults record with no `<Name>`, only `<Property>` children:

| Property | Unit | Range seen | Meaning |
|---|---|---|---|
| `PressPreSilence` | ms | 300–500 | Quiet time before sending a press IR burst |
| `PressInterKey` | ms | 100 | Gap between repeated press bursts |
| `HoldPreSilence` | ms | 50 | Quiet time before sending a hold burst |
| `HoldInterKey` | ms | 100 | Gap between hold repeat bursts |

No per-command timing overrides exist – all commands in a device share these defaults.

**All subsequent entries** are real commands:

The xml config can reference protocols in irProto.bin and commands in SsIr.bin. Index begins at [0]!

| Field | Notes |
|---|---|
| `<Name>` | Command name; must match the suffix in `<ActionId>` references |
| `<Data>/<Protocol>` | Integer index into irProto.bin list. `-1` = escape for ssIr.bin raw command list |
| `<Data>/<Code>` | Hex-encoded command for irproto or integer index for command in raw command list |


#### ssIr

A raw command in ssIr is referenced by inserting the command number like in the following example:

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

#### irProto

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

| Offset |    Bytes     |  Value  |  Meaning|
| ---|---|---|---|
|[0:1]   |    07 00     |  7      |  Protocol index → IrProto.bin|
|[2:3]   |    F4 01     |  500    |  Silence before sending in us |
|[4]     |    03        |  3      |  Repeats|
|[5...n] |              |  Groups |  Always Start, Repeat, Finish |

** Groups **

To encode / decode the group, you need the bit- and quantum count from the corresponding IRproto protocol.

Each group:

```
count            1 byte   number of records in this group (0 = group empty)
record * count   idx (1 byte, section index) + data (bytes, depends on the section)
```

Data bytes of a section = ceil(symbols * bitsPerSymbol / 8), MSB first, symbols back to back, last byte padded with 0 bits. bitsPerSymbol = ceil(log2(number of Encodings)): 2 states = 1 bit, 4 states = 2 bits, 16 states = 4 bits. Sections without payload (repeat frames, 0 bits) have no data bytes.

`0001009C6000`


| Offset |    Bytes     |  Value  |  Meaning|
| ---|---|---|---|
|[+0]     |    00        |  0      |  Empty start group |
|[+1]     |    01        |  1      |  One record in repeat group |
|[+2]     |    00        |  0      |  IRproto section 0 selector |
|[+[3:4]  |    9C60      |  bits   |  IRproto section 0 payload (You need to pull the bit count from irProto to get the required bytes. The bit stream is then MSB aligned.) |
|[+5]     |    00        |  0      |  Empty finish group |
...

** Example protocol that uses data frame = repeat frame (RC5) **

also samsung, japan, and almost all others. Toggle bit is not part of the protocol, not the command.

`0x0700F40103 -->00<-->01009C88<-->00<--`

|Offset    | Bytes      | Value   | Meaning|
|---|---|---|---|
|[5]       | 00         | 0       | Empty start group |
|[6]       | 01         | 1       | One record in repeat group |
|[7]       | 00         | 0       | IRproto section 0 selector. RC5 -> 13 Bits -> 2 Bytes, 3 stuff bits |
|[8:9]     | 9C88       | 0x5009  | Payload
|[10]      | 00         | 0       | Empty finish group |

*** Example protocol that uses special repeat frame (NEC) ***

`0x0000F40101 -->010020DF18E7->0101<-->00<-- `

|Offset    | Bytes      | Value   | Meaning|
|---|---|---|---|
|[5]       | 01         | 1       | One record in start group |
|[6]       | 00         | 0       | IRproto section 0 selector. NEC -> 32 Bits -> 4 Bytes, no stuffing |
|[7...10]  | 20DF18E7   | bits    | Payload
|[11]      | 01         | 1       | One record in repeat group |
|[12]      | 01         | 1       | IRproto section 1 selector. NEC Repeat -> 0 Bits -> Empty |
|[13]      | 00         | 0       | Empty finish group |

*** Example protocol that uses multiple sections (RC6) ***

`0x0200F40103 -->00<-->030070010002B992<-->00<--`

|Offset    | Bytes      | Value   | Meaning|
|---|---|---|---|
|[5]       | 00         | 0       | Empty start group |
|[6]       | 03         | 3       | Three records in repeat group |
|[7]       | 00         | 0       | IRproto section 0 selector. 4 Bits -> 1 Bytes, 4 stuff bits |
|[8]       | 70         | 7       | Payload |
|[10]      | 01         | 1       | IRproto section 1 selector. 1 Bit -> 1 Bytes, 7 stuff bits |
|[11]      | 00         | 0       | Payload |
|[12]      | 02         | 1       | IRproto section 2 selector. 16 Bits -> 2 Bytes, no stuffing |
|[13:14]   | B992       | bits    | Payload |
|[15]      | 00         | 0       | Empty finish group |

*** Example protocol that uses four state coding (Belgacom quad) ***

in this protocol, each timing state encodes four data bits

`0x0300F40101 -->00<-->010022C026F6<-->020122C026F60222C0A6F6`

|Offset    | Bytes      | Value   | Meaning|
|---|---|---|---|
|[5]       | 00         | 0       | Empty start group |
|[6]       | 01         | 1       | One record in repeat group |
|[7]       | 00         | 0       | IRproto section 0 selector. 16 quad bits -> 4 Bytes, no stuffing |
|[8...11]  | 22C026F6   | 0202300002121033 (base 4) | Payload
|[12]      | 02         | 2       | Two records in finish group |
|[13]      | 01         | 1       | IRproto section 1 selector. 16 quad bits -> 4 Bytes, no stuffing |
|[14...17] | 22C026F6   | 0202300002121033 (base 4) | Payload
|[18]      | 02         | 2       | IRproto section 2 selector. 16 quad bits -> 4 Bytes, no stuffing |
|[19...22] | 22C0A6F6   | 0202300022121033 (base 4) | Payload


 *** Example protocol that uses sixteen-state coding ***

No sample available, AI generated!

In this protocol, each timing state encodes 16 data bits.

`0x0300F40101 -->00<-->0100123456789ABCDEF0<-->00`

| Offset | Bytes | Value | Meaning |
|---|---|---|---|
| [5] | 00 | 0 | Empty start group |
| [6] | 01 | 1 | One record in repeat group |
| [7] | 00 | 0 | IRproto section 0 selector |
| [8...15] | 123456789ABCDEF0 | 123456789ABCDEF0 (base 16) | Payload: 16 symbols × 4 bits = 64 bits = 8 bytes |
| [16]     | 00         | 0       | Empty finish group |

---

## Cross-reference summary

| Field in Device | Referenced by |
|---|---|
| `<Id>` | Activity `<Role>/<DeviceId>`, Activity `<Power>/<On>/<Off>`, Action `<Parameter>[0]`, Button `<ActionId>` prefix |
| `<Command>/<Name>` | Button `<ActionId>` middle segment |
| `<State>/<Id>` | Action `<Parameter>[1]` when operation is SetValue/ForceValue |
| `<State>/<Value>` | Action `<Parameter>[2]` when operation is SetValue/ForceValue |

---
