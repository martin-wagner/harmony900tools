#!/usr/bin/env python3
# harmony_init.py — send vendor control transfers to Harmony 900
# Called by udev on plug-in. Runs as root.
# Install: cp harmony_init.py /usr/local/bin/harmony_init.py
#          chmod +x /usr/local/bin/harmony_init.py
#
# requires: pip install pyusb  (or apt install python3-usb)

import usb.core
import usb.util
import sys
import time
import subprocess
import os
import logging

if os.geteuid() != 0:
    print("Run as root: sudo python3 harmony_init.py")
    sys.exit(1)

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s harmony_init %(levelname)s: %(message)s',
    handlers=[
        logging.FileHandler('/var/log/harmony_init.log'),
        logging.StreamHandler(sys.stdout),
    ]
)
log = logging.getLogger()

VID = 0x046d
PID = 0xc11f
IFACE = "usb0"
HOST_IP = "169.254.1.1"

TRANSFERS = [
    (0x05, 0x0201, 0xFEA9),  # device IP = 169.254.1.2
    (0x06, 0xF8FF, 0xFFFF),  # netmask   = 255.255.255.248
    (0x07, 0x0101, 0xFEA9),  # host IP   = 169.254.1.1
]

def run(cmd, check=True):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if check and result.returncode != 0:
        log.error(f"Command failed: {cmd}\n{result.stderr.strip()}")
        sys.exit(1)
    return result

# --- vendor control transfers ---
log.info("Finding Harmony device...")
dev = usb.core.find(idVendor=VID, idProduct=PID)
if dev is None:
    log.error("Device not found")
    sys.exit(1)
log.info(f"Found: {dev.manufacturer} {dev.product}")

if dev.is_kernel_driver_active(0):
    dev.detach_kernel_driver(0)

log.info("Sending vendor control transfers...")
for bRequest, wValue, wIndex in TRANSFERS:
    dev.ctrl_transfer(
        bmRequestType=0x40,
        bRequest=bRequest,
        wValue=wValue,
        wIndex=wIndex,
        data_or_wLength=None
    )
    log.info(f"  bRequest=0x{bRequest:02X} wValue=0x{wValue:04X} wIndex=0x{wIndex:04X} ok")

usb.util.dispose_resources(dev)

# --- reload zaurus driver ---
log.info("Reloading zaurus driver...")
run("modprobe -r zaurus")
run("modprobe zaurus")

# wait for usb0
log.info(f"Waiting for {IFACE}...")
for _ in range(50):
    if run(f"ip link show {IFACE}", check=False).returncode == 0:
        break
    time.sleep(0.25)
else:
    log.error(f"{IFACE} did not appear")
    sys.exit(1)

# assign host IP
run(f"ip addr flush dev {IFACE}", check=False)
run(f"ip addr add {HOST_IP}/24 dev {IFACE}")
run(f"ip link set {IFACE} up")
log.info(f"{IFACE} configured: {HOST_IP}/24")

# restart dnsmasq service so it picks up the live interface
run("systemctl stop harmony-dnsmasq.service", check=False)  # ok if not running
run("systemctl start harmony-dnsmasq.service")
log.info("harmony-dnsmasq.service restarted")
log.info("Init complete")
