import os
import argparse
from lxml import etree

def check_relative_actions(element, file_path):
    for rel in element.xpath(".//RelativeActions"):
        for param in rel.xpath(".//Parameter[@name='State']"):
            print(f"FOUND: {file_path}:{param.sourceline}")

def process_file(file_path):
    print(f"Parsing {file_path}")
    try:
        tree = etree.parse(file_path)
        root = tree.getroot()
        check_relative_actions(root, file_path)
    except Exception as e:
        print(f"Failed: {file_path} ({e})")

def scan_dir(root_dir):
    for dirpath, _, filenames in os.walk(root_dir):
        for name in filenames:
            if name.lower().endswith(".xml"):
                process_file(os.path.join(dirpath, name))

def main():
    parser = argparse.ArgumentParser(
        description="Scan XML files for Parameter[@name='State'] inside RelativeActions"
    )

    parser.add_argument(
        "directory",
        help="Root directory to scan (recursive)"
    )

    args = parser.parse_args()
    scan_dir(args.directory)

if __name__ == "__main__":
    main()
