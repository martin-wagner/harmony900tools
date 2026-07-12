#!/usr/bin/env python3
"""
Compare two XML files for semantic equality, ignoring:
- pretty printing / whitespace-only text nodes
- attribute ordering
- child element ordering (optional, see SORT_CHILDREN)

Usage:
    python3 xml_roundtrip_diff.py original.xml output.xml
"""

import sys
import xml.etree.ElementTree as ET

# Set to True if element order genuinely doesn't matter in your format.
# Harmony configs are mostly order-sensitive (e.g. <Device> lists), so
# default is False; we only ignore ordering for elements you list below.
SORT_CHILDREN_FOR_TAGS = {
    "Properties",  # <Property name="..."> children can be reordered
    "Commands",    # <Command> children can be reordered - matched by <Name>
}

# Fields that are expected to differ (e.g. regenerated timestamps).
# They are still reported, but flagged as EXPECTED rather than counted
# as a round-trip failure.
# Format: (parent_tag, own_tag, attrib_name)
EXPECTED_DIFF_FOR = {
    ("Properties", "Property", "LastUpdated"),
}


def normalize_text(text):
    """Normalize text for comparison: bools are compared case-insensitively."""
    if text.lower() in ("true", "false"):
        return text.lower()
    return text


def is_expected_diff(parent_tag, elem):
    key = (parent_tag, elem.tag, elem.attrib.get("name"))
    return key in EXPECTED_DIFF_FOR


def friendly_label(elem):
    """Best-effort human-readable name for an element, for display only."""
    # Direct <Label> child (Presentation-less case)
    label = elem.find("Label")
    if label is not None and (label.text or "").strip():
        return (label.text or "").strip()
    # <Presentation><Label>
    pres = elem.find("Presentation")
    if pres is not None:
        pres_label = pres.find("Label")
        if pres_label is not None and (pres_label.text or "").strip():
            return (pres_label.text or "").strip()
    # <Name> child (e.g. Command)
    name_child = elem.find("Name")
    if name_child is not None and (name_child.text or "").strip():
        return (name_child.text or "").strip()
    # name attribute
    if "name" in elem.attrib:
        return elem.attrib["name"]
    return None


def strip_whitespace(elem):
    """Remove whitespace-only text/tail so indentation differences don't count."""
    if elem.text is not None and elem.text.strip() == "":
        elem.text = None
    if elem.tail is not None and elem.tail.strip() == "":
        elem.tail = None
    for child in elem:
        strip_whitespace(child)


def identity_text(elem):
    """Best-effort identity string for order-independent matching."""
    if "name" in elem.attrib:
        return elem.attrib["name"]
    # Command elements: identity is their <Name> child text
    name_child = elem.find("Name")
    if name_child is not None:
        return (name_child.text or "").strip()
    # Button-like elements: identity is their <ActionId> child text
    action_id = elem.find("ActionId")
    if action_id is not None and (action_id.text or "").strip():
        return (action_id.text or "").strip()
    return (elem.text or "").strip()


def sort_key(elem):
    attrs = tuple(sorted(elem.attrib.items()))
    return (elem.tag, attrs, identity_text(elem))


def canonicalize(elem):
    strip_whitespace(elem)
    _canonicalize_recursive(elem)


def _canonicalize_recursive(elem):
    for child in elem:
        _canonicalize_recursive(child)
    if elem.tag in SORT_CHILDREN_FOR_TAGS:
        elem[:] = sorted(elem, key=sort_key)


def elem_to_tuple(elem):
    """Convert element to a comparable, order-attribute-independent structure."""
    attrs = tuple(sorted(elem.attrib.items()))
    text = (elem.text or "").strip()
    children = tuple(elem_to_tuple(c) for c in elem)
    return (elem.tag, attrs, text, children)


def diff_elements(path, a, b, diffs, expected_diffs, parent_tag=""):
    a_attrs = dict(a.attrib)
    b_attrs = dict(b.attrib)
    if a.tag != b.tag:
        diffs.append(f"{path}: tag differs: '{a.tag}' vs '{b.tag}'")
        return
    if a_attrs != b_attrs:
        diffs.append(f"{path}: attrs differ: {a_attrs} vs {b_attrs}")

    a_text = (a.text or "").strip()
    b_text = (b.text or "").strip()
    if normalize_text(a_text) != normalize_text(b_text):
        msg = f"{path}: text differs: '{a_text}' vs '{b_text}'"
        if is_expected_diff(parent_tag, a):
            expected_diffs.append(msg)
        else:
            diffs.append(msg)

    a_children = list(a)
    b_children = list(b)

    if a.tag in SORT_CHILDREN_FOR_TAGS:
        # order-independent: match children by identity instead of position
        a_map = {identity_text(c): c for c in a_children}
        b_map = {identity_text(c): c for c in b_children}
        only_in_a = a_map.keys() - b_map.keys()
        only_in_b = b_map.keys() - a_map.keys()
        if only_in_a:
            diffs.append(f"{path}: elements only in original: {sorted(only_in_a)}")
        if only_in_b:
            diffs.append(f"{path}: elements only in output: {sorted(only_in_b)}")
        for key in sorted(a_map.keys() & b_map.keys()):
            label = friendly_label(a_map[key])
            label_str = f" \"{label}\"" if label and label != key else ""
            child_path = f"{path}/{a.tag[:-1] if a.tag.endswith('s') else a.tag}[{key}]{label_str}"
            diff_elements(child_path, a_map[key], b_map[key], diffs, expected_diffs, parent_tag=a.tag)
        return

    if len(a_children) != len(b_children):
        diffs.append(
            f"{path}: child count differs: {len(a_children)} vs {len(b_children)} "
            f"(orig tags={[c.tag for c in a_children]} out tags={[c.tag for c in b_children]})"
        )

    for ca, cb, kind in align_children(a_children, b_children):
        label = friendly_label(ca if ca is not None else cb)
        label_str = f" \"{label}\"" if label else ""
        tag = ca.tag if ca is not None else cb.tag
        if kind == "match":
            child_path = f"{path}/{tag}{label_str}"
            diff_elements(child_path, ca, cb, diffs, expected_diffs, parent_tag=a.tag)
        elif kind == "only_in_a":
            diffs.append(f"{path}/{tag}{label_str}: present in original, missing in output")
        elif kind == "only_in_b":
            diffs.append(f"{path}/{tag}{label_str}: present in output, missing in original")


def child_key(elem):
    """Key used to align children across the two trees: tag + best-effort identity."""
    return (elem.tag, identity_text(elem))


def align_children(a_children, b_children):
    """
    Align two lists of elements using LCS on (tag, identity), so that a single
    inserted/removed element doesn't shift every subsequent element out of
    position (which would otherwise cascade into spurious tag/attr diffs).

    Yields (elem_a, elem_b, kind) tuples where kind is one of:
    - "match": elem_a and elem_b are the aligned pair to diff further
    - "only_in_a": elem_a exists with no counterpart (missing in b)
    - "only_in_b": elem_b exists with no counterpart (missing in a)
    """
    a_keys = [child_key(c) for c in a_children]
    b_keys = [child_key(c) for c in b_children]
    n, m = len(a_children), len(b_children)

    # Standard LCS DP over the keys
    dp = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            if a_keys[i] == b_keys[j]:
                dp[i][j] = dp[i + 1][j + 1] + 1
            else:
                dp[i][j] = max(dp[i + 1][j], dp[i][j + 1])

    result = []
    i = j = 0
    while i < n and j < m:
        if a_keys[i] == b_keys[j]:
            result.append((a_children[i], b_children[j], "match"))
            i += 1
            j += 1
        elif dp[i + 1][j] >= dp[i][j + 1]:
            result.append((a_children[i], None, "only_in_a"))
            i += 1
        else:
            result.append((None, b_children[j], "only_in_b"))
            j += 1
    while i < n:
        result.append((a_children[i], None, "only_in_a"))
        i += 1
    while j < m:
        result.append((None, b_children[j], "only_in_b"))
        j += 1
    return result


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} original.xml output.xml")
        sys.exit(1)

    orig_path, out_path = sys.argv[1], sys.argv[2]

    tree_a = ET.parse(orig_path)
    tree_b = ET.parse(out_path)
    root_a = tree_a.getroot()
    root_b = tree_b.getroot()

    canonicalize(root_a)
    canonicalize(root_b)

    diffs = []
    expected_diffs = []
    diff_elements("", root_a, root_b, diffs, expected_diffs)

    if expected_diffs:
        print(f"{len(expected_diffs)} expected difference(s) (e.g. timestamps):\n")
        for d in expected_diffs:
            print(d)
        print()

    if not diffs:
        print("IDENTICAL (semantically) - round-trip OK")
    else:
        print(f"{len(diffs)} difference(s) found:\n")
        for d in diffs:
            print(d)


if __name__ == "__main__":
    main()
