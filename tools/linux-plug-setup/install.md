# Harmony 900 USB Network — Installation

## Files

- `harmony_init.py` — vendor init script, called on plug-in
- `harmony-dnsmasq.service` — persistent DHCP server for `usb0`
- `99-harmony900.rules` — udev rule to trigger init on plug-in

---

## Install

### 1. Install package dependencies

```bash
sudo apt install dnsmasq python3 python3-usb
```

### 2. Copy init script

```bash
sudo cp harmony_init.py /usr/local/bin/harmony_init.py
sudo chmod +x /usr/local/bin/harmony_init.py
```

### 3. Install systemd service

```bash
sudo cp harmony-dnsmasq.service /etc/systemd/system/harmony-dnsmasq.service
sudo systemctl daemon-reload
```

Optional (not usually needed immediately):

```bash
# start dhcp now (optional, not needed)
# sudo systemctl enable harmony-dnsmasq.service
```

Do **not** start it yet — `harmony_init.py` will start it on first plug-in.

### 4. Install udev rule

```bash
sudo cp 99-harmony900.rules /etc/udev/rules.d/99-harmony900.rules
sudo udevadm control --reload-rules
```

### 5. Tell NetworkManager to leave `usb0` alone

```bash
sudo mkdir -p /etc/NetworkManager/conf.d

cat <<EOF | sudo tee /etc/NetworkManager/conf.d/usb0-unmanaged.conf
[keyfile]
unmanaged-devices=interface-name:usb0
EOF

sudo systemctl restart NetworkManager
```

---

## How It Works on Plug-In

1. udev fires `harmony_init.py`
2. The script sends 3 vendor USB control transfers  
   (IP config to device firmware)
3. The script reloads the `zaurus` driver → `usb0` reappears
4. The script assigns `169.254.1.1/24` to `usb0`
5. The script restarts `harmony-dnsmasq.service`
6. The device sends DHCP DISCOVER → `dnsmasq` replies
7. The device receives `169.254.1.2`
8. `ping 169.254.1.2` works

---

## Logs

```bash
tail -f /var/log/harmony_init.log
tail -f /var/log/harmony-dnsmasq.log
```

---

## Interface Naming / Predictable Network Names

### Will this always appear as `usb0`?

Not guaranteed.

On systems with predictable interface names (common on modern Linux distros), the interface may appear as something like:

```text
enx3e9c253d67e3
```

instead of `usb0`.

The `zaurus` driver typically registers the interface using the pattern `usb%d`, so it will **usually** appear as `usb0`.

However, udev may rename it afterward using MAC-based naming rules.

---

### Check whether renaming rules exist

```bash
ls /etc/udev/rules.d/ /lib/udev/rules.d/ | grep -i net
cat /lib/udev/rules.d/73-usb-net-by-mac.rules 2>/dev/null
```

---

### Recommended: pin interface name to `usb0`

To make the interface name reliable across distros, add a dedicated udev rule:

```bash
# /etc/udev/rules.d/70-harmony900-ifname.rules
ACTION=="add", SUBSYSTEM=="net", \
    ATTRS{idVendor}=="046d", ATTRS{idProduct}=="c11f", \
    NAME="usb0"
```

This rule runs before common MAC-based renaming rules  
(for example `73-usb-net-by-mac.rules`) and forces the interface to remain `usb0`.

Add this alongside the other udev rules during installation.