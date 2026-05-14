# Harmony 900 – Activity Structure Documentation

## Overview

An `<Activity>` defines a usage mode ("Watch TV", "Watch a Movie", "Listen to Radio"). Switching to an activity triggers a coordinated sequence: it powers on/off the relevant devices, sets their inputs, and configures which device handles which function on the remote.

Activities **depend on devices** (via Id references) but not on each other. No activity-to-activity cross-references exist.

One special activity (`PowerOff`, id=`-1`) always exists and turns everything off. It has a stripped-down structure with no Roles or Properties.

---

## Activity types found

| Type | Description |
|---|---|---|
| `PowerOff` | Special: turns all devices off. Id always `-1`. |
| `VirtualCdMulti` | Disc playback (CD 1 ... n) |
| `VirtualDvd` | Disc playback (BD/DVD) |
| `VirtualGameConsole` |Gaming |
| `VirtualGeneric` | Catch-all for activities with no dedicated template |
| `VirtualMusicServer` | ?? |
| `VirtualPvr` | Live TV / satellite viewing / recording |
| `VirtualRadioSimple` |  Radio with simple controls |
| `VirtualSatelliteMusic`  | Satellite radio / music |
| `VirtualTelevisionN` | Live TV / satellite viewing |
| `VirtualVcr` | VHS playback |

This list is most likely incomplete.

The type drives which UI modes, button labels, and properties are applicable. For example, `HideModePlay` and `StopOnExit` only appear on Dvd/Vcr types.

---

## Top-level Tree

```
<Activity>
├── <Id>                   Unique numeric ID; -1 reserved for PowerOff
├── <Type>                 Activity class string
├── <Properties>           UI and behaviour settings  [ABSENT on PowerOff]
│   └── <Property name="..."> × N
├── <Presentation>         UI representation
│   ├── <Label>            Display name
│   ├── <ChannelList/>     Channel/preset list  [OPTIONAL]
│   │       └── <Channel> x N
│   │            ├── <Station> Display name
│   │            ├── <Number>  Channel number
│   │            ├── <Slot>    Position on LCD
│   │            └── <Image>   Channel logo file name for LCD
│   ├── <ControlGroup name="Misc">        Soft-button assignments  [OPTIONAL]
│   │   └── <Button> × N
│   │       ├── <Label>    Button label (may be empty → icon-only)
│   │       ├── <Position> Zero-based display order
│   │       └── <ActionId> <DeviceId>_<CommandName>_Hold"
│   └── <ControlGroup name="HardButtons"> Physical remote button mappings
│           ├── <Label>    Button label (always empty(?))
│           └── <ActionId> "<DeviceId>_<CommandName>_Hold"
├── <EnterActions>         Ordered sequence of actions on activity start
│   └── <Action> × N
│       ├── <Target>       "Device" (only value seen)
│       └── <Operation>
│           ├── <Name>     Operation type
│           └── <Parameter> × N
├── <LeaveActions>         Ordered sequence of actions on activity start
│   └── <Action> × N
│       ├── <Target>       "Device" (only value seen)
│       └── <Operation>
│           ├── <Name>     Operation type
│           └── <Parameter> × N
├── <Role> × N             Functional device assignments  [ABSENT on PowerOff]
│   ├── <Name>             Role type (DEFAULT, DISPLAY, VOLUME, ...)
│   ├── <DeviceId>         References a Device Id
│   └── <Presentation>     Role-specific label overrides (always empty in practice)
└── <Power>                Complete desired power state for all devices
    ├── <On> × N           Device IDs that must be powered On
    └── <Off> × N          Device IDs that must be powered Off
```

---

## Field Details

### `<Id>`

Integer string. `-1` is reserved for the single PowerOff activity. All other activities have a positive Logitech-assigned ID. No local uniqueness constraint observed – IDs are globally unique per user account on the Logitech server.

---

### `<Type>`

See the type table above. The type controls which property keys are valid and which role types are expected. `VirtualGeneric` is used when no specialized template fits.

---

### `<Properties>`

All properties are `<Property name="...">value</Property>`. Absent on `PowerOff`.

#### Universal properties (present in all non-PowerOff activities)

| Property | Values | Meaning |
|---|---|---|
| `ActivityStartPage` | `Transport`, `Numbers`, `GameController` | Which UI tab is shown first when the activity is active. `Transport` = play/pause/skip. `Numbers` = keypad. `GameController` = game buttons. |
| `ControlGroup_Hard Buttons` | `True` | Enables the HardButtons control group in the activity UI. Always `True`. |
| `PowerOffUnusedDevices` | `True` | Remote auto-powers-off devices in the Off list when switching to this activity. Always `True`. |
| `TrainingWheels` | `True` | First-use guidance mode. Always `True` in these configs. |
| `UnusedDevicesHelp` | `False` | Show/hide help for unused devices. Always `False`. |

#### Common optional properties

| Property | Values | Activity types | Meaning |
|---|---|---|---|
| `ActivityStartPage` | `GameController`, `Numbers`, `Transport` | \todo | \todo |
| `ChannelButtonBehaviour` | `BasicChannels` | TV |  How channel up/down work. |
| `ControlGroup_Hard Buttons` | `BasicChannels` | `True` | \todo |
| `ControlGroup_Soft Buttons` | `True` | most | Enables the Misc soft-button panel \todo false? |
| `EnableSmartMenu` | `True` | Dvd | Enable smart disc menu navigation. |
| `EnableSmartZoom` | `True` | Dvd | Enable smart zoom feature. |
| `GuideButtonMode` | `TunerProgramGuide` | Tv |  What the Guide button does. |
| `HideModeControl` | `False` | Dvd, Vcr, GameConsole, TV, Radio | Show/hide the Control mode tab. `False` = visible. |
| `HideModeListen` | `False` | RadioSimple | ?? |
| `HideModeNavigate` | `False` | TV, Dvd, Radio | ?? |
| `HideModePlay` | `False` | Dvd, Vcr | ?? |
| `HideModePlayMode` | `False` | ?? | ?? |
| `HideSurfAllChannels` | `False` | Dvd, Vcr | ?? |
| `HideSurfAllShows` | `False` | Dvd, Vcr | ?? |
| `HideSurfFavoriteChannels` | `False` | ?? | ?? |
| `HideSurfFavoriteShows` | `False` | ?? | ?? |
| `MaxTvContentDays` | 14 | ?? | ?? |
| `MediaButtonMode` | `ShowMedia` | TV, Dvd, Radio | Behaviour of the media buttons. |
| `PlayOnEnter` | `True`, `False` | Dvd, Vcr | Whether to auto-start playback when switching to this activity. |
| `PowerOffUnusedDevices` | `True`, `False` | ?? | ?? |
| `RetainStop` | `False` | ?? | ?? |
| `ScrollChannelsByPage` | `True` | ?? | ?? |
| `ScrollShowsByPage` | `True` | ?? | ?? |
| `StopOnExit` | `True`, `False` | Dvd, Vcr | Whether to send Stop when leaving this activity. |
| `TrainingWheels` | `True`, `False` | ?? |  ?? WTF |
| `UnusedDevicesHelp` | `False` | ?? | ?? |

---

### `<Presentation>`

Same structure as device Presentation.

- `<Label>`: user-defined display name.
- `<ChannelList/>`: Channel / preset list for LCD. Includes Channel number and Channel icon file name for LCD.
- `<ControlGroup name="Misc">`: soft buttons for this activity's UI. Absent on `PowerOff`
- `<ControlGroup name="HardButtons">`: physical remote button layout for this activity.

---

### `<EnterActions>`

An ordered sequence of `<Action>` elements executed top-to-bottom when switching to this activity. Three operation types are used:

| Operation | Parameters | Meaning |
|---|---|---|
| `SetValue` | `deviceId`, `stateName`, `value` | Update the internal state model. If the target state has matching DiscreteActions, those are triggered automatically (which in turn sends the IR commands). |
| `SendCommand` | `deviceId`, `commandName`, `"Press"` | Send an IR command directly to the device, bypassing the state machine. |
| `SendDelay` | `deviceId`, `delayMs` | Wait N milliseconds before executing the next action. The deviceId in a SendDelay is a reference but has no operational meaning – the delay is a global pause. |

#### Typical startup sequence pattern

```
1. SetValue(device, Input, targetInput)      → updates state, sends input-select IR
2. SetValue(receiver, Input, receiverInput)  → sets AV receiver input
3. SendCommand(tv, PowerOn, Press)           → explicit power-on (belt+suspenders)
4. SendDelay(tv, 5000–10000)                → wait for TV to boot
5. SendDelay(tv, 2000)                       → additional settling time
6. SendCommand(tv, InputHdmiX, Press)        → confirm input again after boot
```

The `SetValue` for inputs fires DiscreteActions which send IR.

In theory, `<Power>` and SetValue startup state machine should make your setup ready-to-operate. In practice, this can fail. For commands that don't toggle/use sequences, sending them a second time makes this more robust against errors.

---

### `<LeaveActions>`

same as `<EnterActions>`, just when leaving the activity.

### `<Role>`

Roles bind a functional slot to a specific device. The remote firmware uses roles to know which device receives button presses in a given activity.

| Role Name | Meaning | Notes |
|---|---|---|
| `DEFAULT` | Main controlled device – receives all unassigned button presses | Always present in non-PowerOff activities |
| `DISPLAY` | Screen / TV | Optional. Used when the display is separate from DEFAULT. |
| `VOLUME` | Handles volume up/down/mute | Optional. If absent, volume goes to DEFAULT. |
| `LIGHTCONTROL` | Primary ambient lighting device | Optional. |
| `LIGHTCONTROL2` | Secondary lighting device | Optional. Seen in `Listen to Radio`. |
| `PASSTHROUGH` | Additional device that also receives context-relevant commands | Optional. |
| `PASSTHROUGH2`…`PASSTHROUGH4` | Further passthrough devices | Optional, sequentially numbered. |

The `<Presentation>` inside each Role is always empty in practice (null labels). It appears to be a placeholder for role-specific label overrides. \todo korrekt?

**A device can hold multiple roles simultaneously.** Examples:
- `Listen to Radio`: AV Receiver is both `DEFAULT` and `VOLUME` – it is the primary device *and* handles its own volume.
- `Radio Sat`, `PC`: `Light Controller` is both `DEFAULT` and `LIGHTCONTROL`. This happens when there is no media device with discrete remote control; the light controller becomes the nominal default.

---

### `<Power>`

This section is a **complete declarative power map for all devices in the configuration**. Every device in the file appears in exactly one of `<On>` or `<Off>` for every activity. No device is ever missing, no extra device appears.

```
<Power>
  <On>deviceId</On>   × N    → these devices must be powered On
  <Off>deviceId</Off> × N    → these devices must be powered Off
```

**Switching logic:** When the user switches from activity A to activity B, the runtime compares the two Power sections:

- Devices in B.On that were in A.Off → power them on (send power-on command or trigger Power state transition).
- Devices in A.On that are in B.Off → power them off (send power-off command).
- Devices that are On in both A and B → leave running (no power command sent).

Then the B.EnterActions sequence is executed to set inputs and modes.

`PowerOff` has `<On>` empty and all devices in `<Off>`. It uses `SetValue(deviceId, Power, Off)` in EnterActions to explicitly force each device off through the state machine.

---

## Activity State Model (Power/Enter/Exit lifecycle)

The Harmony doesn't store an explicit "current activity" state in the XML – that lives in the remote's runtime. But the structure implies three lifecycle phases:

**1. Entering (EnterActions)**
Runs on switch-in. Sets device states and sends IR sequences. Power-on is handled by the Power section diff logic before EnterActions runs.

**2. Active**
The Role assignments determine which device receives button presses. The HardButtons and Misc control groups define the button layout. The device's own state machine tracks input/mode changes that happen while the activity is active.

**3. Leaving (LeaveActions)**
Runs on switch-out. Tthe next activity's Power section diff determines what to power off, and `StopOnExit`/`PlayOnEnter` \todo really? properties handle playback state.

---

## Cross-dependencies

### Activity → Device references

| Activity field | References |
|---|---|
| `<Role>/<DeviceId>` | Device `<Id>` |
| `<Power>/<On>` / `<Power>/<Off>` | Device `<Id>` |
| `<EnterActions>/.../Parameter[0]` | Device `<Id>` |
| `<EnterActions>/.../Parameter[1]` | Device `<State>/<Id>` (when SetValue) |
| `<EnterActions>/.../Parameter[2]` | Device `<State>/<Value>` (when SetValue) or Command `<Name>` (when SendCommand) |

### Activity → Activity references

None. Activities are fully self-contained. No EnterAction or other field references another activity's ID.

---

## Notable findings

**Power section is exhaustive and mandatory.** Every device in the config must be listed in every activity's Power section. The structure enforces a total desired-state declaration, not a delta. This means adding a new device requires updating every activity's Power section.

**DEFAULT role is arbitrary** Any device can be the DEFAULT device when there is no discrete-IR-controllable media device to take the role. This means unassigned button presses go to this device, which might be a no-op for most buttons.

**SendDelay device parameter is decorative.** The first parameter of `SendDelay` is always a device ID, but it has no operational meaning. The delay is a global pause in the sequence. This is consistent across all activities.

**AlwaysOn device in Off list.** If a device has `AlwaysOn=true` at the device level but can be listed in the Off list. \todo check this... The device property is a hint to the setup wizard, not a runtime constraint. The Power section in the activity is always authoritative.
