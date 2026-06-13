"""
harmony_accumulate.py

Scans a folder recursively for Logitech Harmony UserConfiguration XML files,
extracts all enumerable, key-value, and structured items, and writes one
output file per item type into an 'accumulated/' subfolder.

Usage:
    python harmony_accumulate.py <folder>

Output files (in <folder>/accumulated/):

  Simple enumerations (sorted unique values):
    device_types.txt
    activity_types.txt
    role_names.txt
    state_ids.txt
    state_values.txt
    operation_names.txt
    action_targets.txt
    discrete_action_tags.txt
    relative_action_tags.txt
    controlgroup_names.txt

  Key → value maps (property name: sorted unique values):
    device_properties.txt
    activity_properties.txt

  Complex structured items (with source info):
    device_states.txt
    device_numeric.txt
    device_command_timing.txt
    activity_enter_actions.txt
    activity_roles.txt
    activity_power.txt

  New items:
    device_ids.txt
        Unique device ID numbers across all files (simple enum).

    device_hardbutton_labels.txt
        All button Label values from ControlGroup name="HardButtons" (simple enum).

    device_hardbutton_labels_nonempty.txt
        HardButton labels that are non-empty, with source info.
        Empty file if none found.

    device_actionid_no_hold.txt
        ActionIds from HardButtons and Misc groups where the suffix is NOT _Hold,
        with source info. Empty file if none found.

    activity_hardbutton_labels_nonempty.txt
        HardButton labels that are non-empty in activities, with source info.
        Empty file if none found.

    activity_actionid_no_hold.txt
        ActionIds from activity HardButtons and Misc groups without _Hold suffix.
        Empty file if none found.

    activity_role_presentation_nonempty.txt
        Role <Presentation> elements that contain non-empty content, with source.
        Empty file if none found.

    activity_power_missing_devices.txt
        Activities where the Power section does not cover all devices in the file.
        Empty file if all Power sections are complete.

    device_states_both_action_types.txt
        States that contain both <DiscreteActions> and <RelativeActions>.
        Each entry shows source, state id, values, and the child action tags
        of each block (no action detail). Empty file if no such states exist.
"""

import sys
import os
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def find_config_files(root_folder: str) -> list[Path]:
    """Recursively find all UserConfiguration*.xml files."""
    result = []
    for p in Path(root_folder).rglob("*.xml"):
        if "userconfiguration" in p.name.lower():
            result.append(p)
    return sorted(result)


def source_tag(filepath: Path, element, label_path: str = None) -> str:
    """Build a readable source label: filename + optional element label."""
    parts = [filepath.name]
    if label_path and element is not None:
        label = element.findtext(label_path)
        if label:
            parts.append(label)
        elem_id = element.findtext("Id")
        if elem_id:
            parts.append(f"id={elem_id}")
    return " | ".join(parts)


def write_enum(path: Path, values: set):
    """Write a sorted unique-value list."""
    with open(path, "w", encoding="utf-8") as f:
        for v in sorted(values):
            f.write(v + "\n")


def write_kv_map(path: Path, mapping: dict):
    """Write a property-name → sorted-values mapping."""
    with open(path, "w", encoding="utf-8") as f:
        for key in sorted(mapping.keys()):
            values = sorted(mapping[key])
            f.write(f"{key}:\n")
            for v in values:
                f.write(f"    {v}\n")
            f.write("\n")


def write_complex(path: Path, entries: list[str]):
    """Write a list of pre-formatted complex entries separated by blank lines."""
    with open(path, "w", encoding="utf-8") as f:
        for entry in entries:
            f.write(entry)
            f.write("\n")


# ---------------------------------------------------------------------------
# Accumulators
# ---------------------------------------------------------------------------

class Accumulator:
    def __init__(self):
        # --- simple enumerations ---
        self.device_types: set = set()
        self.activity_types: set = set()
        self.role_names: set = set()
        self.state_ids: set = set()
        self.state_values: set = set()
        self.operation_names: set = set()
        self.action_targets: set = set()
        self.discrete_action_tags: set = set()
        self.relative_action_tags: set = set()
        self.controlgroup_names: set = set()
        self.button_names: set = set()

        # --- key → values maps ---
        self.device_properties: dict = defaultdict(set)
        self.activity_properties: dict = defaultdict(set)

        # --- complex items (list of formatted strings) ---
        self.device_states: list = []
        self.device_numeric: list = []
        self.device_command_timing: list = []
        self.activity_enter_actions: list = []
        self.activity_roles: list = []
        self.activity_power: list = []

        # --- new items ---
        self.device_ids: set = set()
        self.device_hardbutton_labels: set = set()
        self.device_hardbutton_labels_nonempty: list = []
        self.device_actionid_no_hold: list = []
        self.activity_hardbutton_labels_nonempty: list = []
        self.activity_actionid_no_hold: list = []
        self.activity_role_presentation_nonempty: list = []
        self.activity_power_missing_devices: list = []

        # States that contain both DiscreteActions and RelativeActions
        self.device_states_both_action_types: list = []

        # All Parameter name= values found inside any Action/Operation
        self.action_parameter_names: set = set()

    # -----------------------------------------------------------------------
    # Device processing
    # -----------------------------------------------------------------------

    def process_device(self, dev, filepath: Path):
        src = source_tag(filepath, dev, "Presentation/Label")

        # Type
        t = dev.findtext("Type")
        if t:
            self.device_types.add(t)

        # Properties
        for p in dev.findall("Properties/Property"):
            name = p.get("name")
            value = (p.text or "").strip()
            if name:
                self.device_properties[name].add(value)

        # ControlGroups
        for cg in dev.findall("Presentation/ControlGroup"):
            name = cg.get("name")
            if name:
                self.controlgroup_names.add(name)

            for btn in cg.findall("Button"):
                btn_name = btn.get("name")
                if btn_name:
                    self.button_names.add(btn_name)

        # States
        self._process_device_states(dev, filepath, src)

        # Numeric
        self._process_device_numeric(dev, filepath, src)

        # Command timing
        self._process_device_command_timing(dev, filepath, src)

        # New: device ID
        dev_id = dev.findtext("Id")
        if dev_id:
            self.device_ids.add(dev_id)

        # New: HardButton labels + ActionId suffix check
        self._process_device_buttons(dev, filepath, src)

    def _process_device_states(self, dev, filepath: Path, src: str):
        states = dev.findall("States/State")
        if not states:
            return

        lines = [f"[{src}]"]
        for st in states:
            st_id = st.findtext("Id") or ""
            self.state_ids.add(st_id)

            values = [v.text for v in st.findall("Value") if v.text]
            for v in values:
                self.state_values.add(v)

            delay = st.findtext("Delay") or ""
            lines.append(f"  State: {st_id}")
            lines.append(f"    Values: {values}")
            if delay:
                lines.append(f"    Delay: {delay} ms")

            # DiscreteActions
            da = st.find("DiscreteActions")
            if da is not None:
                for item in da:
                    self.discrete_action_tags.add(item.tag)
                    target_value = item.findtext("Name") or ""
                    action = item.find("Action")
                    op_str = self._format_action(action)
                    lines.append(f"    {item.tag} name={target_value!r}: {op_str}")

            # RelativeActions
            ra = st.find("RelativeActions")
            if ra is not None:
                for item in ra:
                    self.relative_action_tags.add(item.tag)
                    action = item.find("Action")
                    op_str = self._format_action(action)
                    lines.append(f"    {item.tag}: {op_str}")

            # Record states that carry both action types
            if da is not None and ra is not None:
                entry = (
                    f"[{src}]\n"
                    f"  State:             {st_id}\n"
                    f"  Values:            {values}\n"
                    f"  DiscreteActions:   {[c.tag for c in da]}\n"
                    f"  RelativeActions:   {[c.tag for c in ra]}\n"
                )
                self.device_states_both_action_types.append(entry)

        lines.append("")
        self.device_states.append("\n".join(lines))

    def _process_device_numeric(self, dev, filepath: Path, src: str):
        num = dev.find("Numeric")
        if num is None:
            return

        fixed = num.findtext("FixedDigits") or "0"
        has_finish = num.find("Finish") is not None
        has_first = num.find("FirstDigit") is not None
        has_middle = num.find("MiddleDigit") is not None
        has_last = num.find("LastDigit") is not None

        # Collect Finish action
        finish_action = ""
        finish_elem = num.find("Finish")
        if finish_elem is not None:
            action = finish_elem.find("Action")
            finish_action = self._format_action(action)

        # Collect digit 0..9 commands for FirstDigit
        digit_cmds = []
        first = num.find("FirstDigit")
        if first is not None:
            for i, digit in enumerate(first.findall("Digit")):
                action = digit.find("Action")
                digit_cmds.append(f"    Digit[{i}]: {self._format_action(action)}")

        lines = [
            f"[{src}]",
            f"  FixedDigits: {fixed}",
            f"  Sections: FirstDigit={has_first}  MiddleDigit={has_middle}"
            f"  LastDigit={has_last}  Finish={has_finish}",
        ]
        if finish_action:
            lines.append(f"  Finish action: {finish_action}")
        if digit_cmds:
            lines.append("  FirstDigit commands:")
            lines.extend(digit_cmds)
        lines.append("")
        self.device_numeric.append("\n".join(lines))

    def _process_device_buttons(self, dev, filepath: Path, src: str):
        """Collect HardButton labels and check ActionId suffixes for Misc+HardButtons."""
        # HardButton labels (enum + nonempty-with-source)
        hb_nonempty = []
        hb_cg = dev.find("Presentation/ControlGroup[@name='HardButtons']")
        if hb_cg is not None:
            for btn in hb_cg.findall("Button"):
                label = (btn.findtext("Label") or "").strip()
                self.device_hardbutton_labels.add(label)
                if label:
                    pos = btn.findtext("Position") or "?"
                    hb_nonempty.append(f"  pos={pos}  label={label!r}")

        if hb_nonempty:
            self.device_hardbutton_labels_nonempty.append(
                f"[{src}]\n" + "\n".join(hb_nonempty) + "\n"
            )

        # ActionId suffix check across both HardButtons and Misc
        no_hold = []
        for cg_name in ("HardButtons", "Misc"):
            cg = dev.find(f"Presentation/ControlGroup[@name='{cg_name}']")
            if cg is None:
                continue
            for btn in cg.findall("Button"):
                action_id = (btn.findtext("ActionId") or "").strip()
                if not action_id:
                    continue
                if not action_id.endswith("_Hold"):
                    label = (btn.findtext("Label") or "").strip()
                    pos = btn.findtext("Position") or "?"
                    no_hold.append(
                        f"  group={cg_name}  pos={pos}  label={label!r}"
                        f"  actionId={action_id!r}"
                    )

        if no_hold:
            self.device_actionid_no_hold.append(
                f"[{src}]\n" + "\n".join(no_hold) + "\n"
            )

    def _process_device_command_timing(self, dev, filepath: Path, src: str):
        cmds = dev.find("Commands")
        if cmds is None:
            return
        all_cmds = list(cmds)
        if not all_cmds:
            return

        # First entry = timing defaults (no Name child, only Property children)
        timing_entry = all_cmds[0]
        props = {p.get("name"): (p.text or "").strip()
                 for p in timing_entry.findall("Property")}
        if not props:
            return

        cmd_count = sum(1 for c in all_cmds[1:] if c.findtext("Name"))

        lines = [
            f"[{src}]",
            f"  PressPreSilence:  {props.get('PressPreSilence', '?')} ms",
            f"  PressInterKey:    {props.get('PressInterKey', '?')} ms",
            f"  HoldPreSilence:   {props.get('HoldPreSilence', '?')} ms",
            f"  HoldInterKey:     {props.get('HoldInterKey', '?')} ms",
            f"  Command count:    {cmd_count}",
            "",
        ]
        self.device_command_timing.append("\n".join(lines))

    # -----------------------------------------------------------------------
    # Activity processing
    # -----------------------------------------------------------------------

    def process_activity(self, act, filepath: Path, file_device_ids: set):
        src = source_tag(filepath, act, "Presentation/Label")

        # Type
        t = act.findtext("Type")
        if t:
            self.activity_types.add(t)

        # Properties
        for p in act.findall("Properties/Property"):
            name = p.get("name")
            value = (p.text or "").strip()
            if name:
                self.activity_properties[name].add(value)

        # ControlGroups
        for cg in act.findall("Presentation/ControlGroup"):
            name = cg.get("name")
            if name:
                self.controlgroup_names.add(name)

            for btn in cg.findall("Button"):
                btn_name = btn.get("name")
                if btn_name:
                    self.button_names.add(btn_name)

        # EnterActions
        self._process_activity_enter_actions(act, filepath, src)

        # Roles
        self._process_activity_roles(act, filepath, src)

        # Power
        self._process_activity_power(act, filepath, src, file_device_ids)

        # New: activity button labels + ActionId suffix check
        self._process_activity_buttons(act, filepath, src)

        # New: role presentation non-empty check
        self._process_activity_role_presentation(act, filepath, src)

    def _process_activity_enter_actions(self, act, filepath: Path, src: str):
        actions = act.findall("EnterActions/Action")
        if not actions:
            return

        lines = [f"[{src}]"]
        for action in actions:
            lines.append(f"  {self._format_action(action)}")
        lines.append("")
        self.activity_enter_actions.append("\n".join(lines))

    def _process_activity_roles(self, act, filepath: Path, src: str):
        roles = act.findall("Role")
        if not roles:
            return

        lines = [f"[{src}]"]
        for role in roles:
            rname = role.findtext("Name") or ""
            rdev = role.findtext("DeviceId") or ""
            self.role_names.add(rname)
            lines.append(f"  {rname}: DeviceId={rdev}")
        lines.append("")
        self.activity_roles.append("\n".join(lines))

    def _process_activity_power(self, act, filepath: Path, src: str, file_device_ids: set):
        power = act.find("Power")
        if power is None:
            return

        ons = [e.text for e in power.findall("On") if e.text]
        offs = [e.text for e in power.findall("Off") if e.text]

        lines = [
            f"[{src}]",
            f"  On:  {ons}",
            f"  Off: {offs}",
            "",
        ]
        self.activity_power.append("\n".join(lines))

        # Completeness check: every device in this file must appear in On or Off
        if file_device_ids:
            covered = set(ons) | set(offs)
            missing = file_device_ids - covered
            if missing:
                self.activity_power_missing_devices.append(
                    f"[{src}]\n"
                    f"  Missing device IDs: {sorted(missing)}\n"
                )

    def _process_activity_buttons(self, act, filepath: Path, src: str):
        """Collect activity HardButton labels and check ActionId suffixes."""
        hb_nonempty = []
        hb_cg = act.find("Presentation/ControlGroup[@name='HardButtons']")
        if hb_cg is not None:
            for btn in hb_cg.findall("Button"):
                label = (btn.findtext("Label") or "").strip()
                if label:
                    pos = btn.findtext("Position") or "?"
                    hb_nonempty.append(f"  pos={pos}  label={label!r}")

        if hb_nonempty:
            self.activity_hardbutton_labels_nonempty.append(
                f"[{src}]\n" + "\n".join(hb_nonempty) + "\n"
            )

        # ActionId suffix check across HardButtons and Misc
        no_hold = []
        for cg_name in ("HardButtons", "Misc"):
            cg = act.find(f"Presentation/ControlGroup[@name='{cg_name}']")
            if cg is None:
                continue
            for btn in cg.findall("Button"):
                action_id = (btn.findtext("ActionId") or "").strip()
                if not action_id:
                    continue
                if not action_id.endswith("_Hold"):
                    label = (btn.findtext("Label") or "").strip()
                    pos = btn.findtext("Position") or "?"
                    no_hold.append(
                        f"  group={cg_name}  pos={pos}  label={label!r}"
                        f"  actionId={action_id!r}"
                    )

        if no_hold:
            self.activity_actionid_no_hold.append(
                f"[{src}]\n" + "\n".join(no_hold) + "\n"
            )

    def _process_activity_role_presentation(self, act, filepath: Path, src: str):
        """Collect Role <Presentation> elements that contain non-empty content."""
        nonempty = []
        for role in act.findall("Role"):
            rname = role.findtext("Name") or ""
            pres = role.find("Presentation")
            if pres is None:
                continue
            # Non-empty means: has child elements, or non-blank text content
            has_children = len(list(pres)) > 0
            has_text = (pres.text or "").strip() != ""
            if has_children or has_text:
                import xml.etree.ElementTree as _ET
                pres_str = _ET.tostring(pres, encoding="unicode").strip()
                nonempty.append(f"  role={rname!r}  presentation={pres_str}")

        if nonempty:
            self.activity_role_presentation_nonempty.append(
                f"[{src}]\n" + "\n".join(nonempty) + "\n"
            )

    # -----------------------------------------------------------------------
    # Shared helpers
    # -----------------------------------------------------------------------

    def _format_action(self, action) -> str:
        """Format an <Action> element to a readable one-liner."""
        if action is None:
            return "(no action)"
        target = action.findtext("Target") or ""
        if target:
            self.action_targets.add(target)
        op = action.find("Operation")
        if op is None:
            return f"target={target!r} (no operation)"
        op_name = op.findtext("Name") or ""
        if op_name:
            self.operation_names.add(op_name)
        params = [p.text for p in op.findall("Parameter") if p.text]
        for p in op.findall("Parameter"):
            name_attr = p.get("name")
            if name_attr:
                self.action_parameter_names.add(name_attr)
        return f"target={target!r}  op={op_name}  params={params}"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def process_file(filepath: Path, acc: Accumulator):
    try:
        tree = ET.parse(filepath)
    except ET.ParseError as e:
        print(f"  WARNING: could not parse {filepath.name}: {e}")
        return

    root = tree.getroot()

    # Collect device IDs for this file first (needed for power completeness check)
    file_device_ids = set()
    for dev in root.findall("Device"):
        dev_id = dev.findtext("Id")
        if dev_id:
            file_device_ids.add(dev_id)

    for dev in root.findall("Device"):
        acc.process_device(dev, filepath)

    for act in root.findall("Activity"):
        acc.process_activity(act, filepath, file_device_ids)


def write_outputs(out_dir: Path, acc: Accumulator):
    out_dir.mkdir(parents=True, exist_ok=True)

    # --- simple enumerations ---
    write_enum(out_dir / "device_types.txt",         acc.device_types)
    write_enum(out_dir / "activity_types.txt",        acc.activity_types)
    write_enum(out_dir / "role_names.txt",            acc.role_names)
    write_enum(out_dir / "state_ids.txt",             acc.state_ids)
    write_enum(out_dir / "state_values.txt",          acc.state_values)
    write_enum(out_dir / "operation_names.txt",          acc.operation_names)
    write_enum(out_dir / "action_targets.txt",           acc.action_targets)
    write_enum(out_dir / "discrete_action_tags.txt",     acc.discrete_action_tags)
    write_enum(out_dir / "action_parameter_names.txt",   acc.action_parameter_names)
    write_enum(out_dir / "relative_action_tags.txt",  acc.relative_action_tags)
    write_enum(out_dir / "controlgroup_names.txt",    acc.controlgroup_names)

    # --- key → values maps ---
    write_kv_map(out_dir / "device_properties.txt",   acc.device_properties)
    write_kv_map(out_dir / "activity_properties.txt", acc.activity_properties)

    # --- complex structured items ---
    write_complex(out_dir / "device_states.txt",          acc.device_states)
    write_complex(out_dir / "device_numeric.txt",          acc.device_numeric)
    write_complex(out_dir / "device_command_timing.txt",   acc.device_command_timing)
    write_complex(out_dir / "activity_enter_actions.txt",  acc.activity_enter_actions)
    write_complex(out_dir / "activity_roles.txt",          acc.activity_roles)
    write_complex(out_dir / "activity_power.txt",          acc.activity_power)

    # --- new items ---
    write_enum(out_dir / "device_ids.txt",                          acc.device_ids)
    write_enum(out_dir / "device_hardbutton_labels.txt",            acc.device_hardbutton_labels)
    write_complex(out_dir / "device_hardbutton_labels_nonempty.txt", acc.device_hardbutton_labels_nonempty)
    write_complex(out_dir / "device_actionid_no_hold.txt",           acc.device_actionid_no_hold)
    write_complex(out_dir / "activity_hardbutton_labels_nonempty.txt", acc.activity_hardbutton_labels_nonempty)
    write_complex(out_dir / "activity_actionid_no_hold.txt",          acc.activity_actionid_no_hold)
    write_complex(out_dir / "activity_role_presentation_nonempty.txt", acc.activity_role_presentation_nonempty)
    write_complex(out_dir / "activity_power_missing_devices.txt",     acc.activity_power_missing_devices)
    write_enum(out_dir / "button_names.txt",          acc.button_names)
    write_complex(out_dir / "device_states_both_action_types.txt",    acc.device_states_both_action_types)


def main():
    if len(sys.argv) != 2:
        print("Usage: python harmony_accumulate.py <folder>")
        sys.exit(1)

    folder = sys.argv[1]
    if not os.path.isdir(folder):
        print(f"Error: {folder!r} is not a directory")
        sys.exit(1)

    files = find_config_files(folder)
    if not files:
        print(f"No UserConfiguration*.xml files found in {folder!r}")
        sys.exit(1)

    print(f"Found {len(files)} config file(s):")
    for f in files:
        print(f"  {f}")

    acc = Accumulator()
    for filepath in files:
        print(f"Processing: {filepath.name}")
        process_file(filepath, acc)

    out_dir = Path(folder) / "accumulated"
    write_outputs(out_dir, acc)
    print(f"\nOutput written to: {out_dir}/")

    # Summary
    print("\nFiles written:")
    for f in sorted(out_dir.iterdir()):
        size = f.stat().st_size
        lines = f.read_text(encoding="utf-8").count("\n")
        print(f"  {f.name:<40}  {lines:>4} lines  ({size} bytes)")


if __name__ == "__main__":
    main()