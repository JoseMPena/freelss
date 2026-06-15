# FreeLSS

FreeLSS is a laser scanning program for the Raspberry Pi. It allows a Raspberry Pi to function as the core to a complete turn table laser scanning system.

External hardware (turntable stepper, two laser modules and the lighting strip) is driven by an **Arduino Mega running the FreeLSS G-code firmware over USB**, not by the Pi's GPIO pins. The Pi only sends one G-code command per movement and waits for the firmware's `>` ready ack.

---

## Supported platform


| Layer        | Specification                                                                                                                                       |
| ------------ | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| SBC          | Raspberry Pi 3 B+ (BCM2837, quad-core Cortex-A53, 1 GB RAM)                                                                                         |
| OS           | **FabScan Pi OS image** (Raspbian Bullseye, `armhf`, legacy MMAL stack), see [FabScan Pi OS (Raspbian Bullseye)](#fabscan-pi-os-raspbian-bullseye). |
| Architecture | `armv7l` — verify with `uname -m`                                                                                                                   |
| Camera       | Pi Camera Module v1/v2 (CSI, legacy MMAL stack)                                                                                                     |
| Arduino      | Mega with FabscanPi G-code firmware over USB                                                                                                        |


---

## FabScan Pi OS (Raspbian Bullseye)

Build guide for the [FabScan Pi OS image](https://fabscan.org/downloads/) — Raspbian Bullseye on `armhf` with the legacy MMAL camera stack. FreeLSS on this platform uses `libraspberrypi-dev` and `/opt/vc` paths, not libcamera.

### 1. Fix apt (Bullseye moved to `oldoldstable`)

FabScan Pi images ship with Bullseye apt sources. The suite was reclassified from `stable` to `oldoldstable`; stale package indexes then request removed package versions (e.g. `git-man 1:2.30.2-1` → 404).

```bash
# Inspect sources — Bullseye stays on the main mirror (not legacy.raspbian.org)
grep -rE 'raspbian|raspberrypi' /etc/apt/sources.list /etc/apt/sources.list.d/
```

Expected Raspbian line:

```
deb http://raspbian.raspberrypi.org/raspbian/ bullseye main contrib non-free rpi
```

Expected Pi-specific line:

```
deb http://archive.raspberrypi.org/debian bullseye main
```

Refresh indexes and accept the suite change:

```bash
sudo apt update --allow-releaseinfo-change
```

If 404 errors persist, clear cached lists and update again:

```bash
sudo rm -rf /var/lib/apt/lists/*
sudo apt update --allow-releaseinfo-change
```

Verify apt is healthy:

```bash
sudo apt install -y git
apt-cache policy git-man   # should show 1:2.30.2-1+deb11u5 or newer, not 1:2.30.2-1
```

> **Do not** point Bullseye at `legacy.raspbian.org` — that host is for EOL releases (Buster and older).

### 2. Install build dependencies

```bash
sudo apt install -y \
  build-essential gcc g++ make git \
  libpng-dev libjpeg-dev unzip \
  sqlite3 libsqlite3-dev \
  libmicrohttpd-dev libcurl4-openssl-dev libiw-dev libssl-dev \
  libraspberrypi-dev libraspberrypi0
```

### 3. MMAL headers (`mmal/mmal.h`)

FreeLSS expects the old Pi userland layout under `/opt/vc`. On Bullseye, headers and libraries live under `/usr` via `libraspberrypi-dev`. Bridge the paths with symlinks:

```bash
sudo mkdir -p /opt/vc
sudo ln -sfn /usr/include /opt/vc/include
sudo ln -sfn /usr/lib/arm-linux-gnueabihf /opt/vc/lib
```

Verify:

```bash
ls /usr/include/interface/mmal/mmal.h
```

### 4. wiringPi (GPIO status LED / scan button)

`wiringPi` is not packaged in Bullseye apt. Build and install from source:

```bash
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
./build
sudo ./build
```

Verify (the FreeLSS Makefile already adds `-I/usr/local/include` and `-L/usr/local/lib`):

```bash
ls /usr/local/include/wiringPi.h
ls /usr/local/lib/libwiringPi.so
```

> To run without GPIO hardware, compile with `-DMOCK` in `src/Makefile` `CFLAGS` and remove `-lwiringPi` from `LFLAGS` instead of installing wiringPi.

### 5. Enable legacy camera stack

FreeLSS uses MMAL, not libcamera. On Bullseye the legacy stack must be enabled explicitly:

```bash
sudo raspi-config nonint do_legacy 0
sudo reboot
```

After reboot:

```bash
vcgencmd get_camera   # supported=1 detected=1
```

### 6. Build and run

```bash
git clone https://github.com/JoseMPena/freelss.git
cd freelss
make
cd src && sudo ./freelss
```

FreeLSS binds port 80 by default. As we're using FabScanPi OS only for the image support, we can safely remove it:

```bash
sudo systemctl stop fabscanpi-server
sudo apt-get remove fabscanpi-server
sudo apt-get remove haproxy
```

### 7. Auto-start on boot

```bash
make startup
```

---

## Recommended presets (Pi 3 B+ / DietPi 64-bit)


| Use case             | Preset       | Resolution | Notes                       |
| -------------------- | ------------ | ---------- | --------------------------- |
| Default scan         | 1.2 MP video | 1280×960   | Recommended for reliability |
| Calibration          | VGA          | 640×480    | Fast turnaround             |
| High detail (OV5647) | 5 MP still   | 2592×1944  | ~360 ms laser delay         |
| High detail (IMX219) | 8 MP still   | 3296×2512  | ~360 ms laser delay         |


---

## Troubleshooting


| Symptom                                            | Fix                                                                                                     |
| -------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| `libcamera not found` at build                     | `sudo apt install libcamera-dev`                                                                        |
| `rpicam-hello`: no cameras listed                  | `dietpi-config` → enable RPi Camera; add `camera_auto_detect=1` to `/boot/config.txt`                   |
| Wrong arch or MMAL build errors                    | Confirm 64-bit DietPi image: `uname -m` must show `aarch64`; use `libcamera_port` branch                |
| Mock camera fallback in logs                       | Run `./scripts/verify-camera.sh`; check `dtoverlay` in `/boot/config.txt`                               |
| Laser line missing in scan                         | Increase laser delay in Scanner.cpp `m_laserDelaySec` by 0.05 s increments                              |
| OOM at 5 MP                                        | `free -h`; fall back to 1.2 MP preset                                                                   |
| `Not enough buffers` / camera start failed at 5 MP | Settings → GPU Memory **256** (or 320) MB, save, **reboot**; restart `freelss` after failed camera init |
| wiringPi link error                                | Build from `WiringPi/WiringPi` community fork (not the original abandoned repo)                         |
| Motor does not rotate                              | Settings → **Test Arduino Connection**; `tail -f /var/lib/freelss/freelss.log`; see below               |
| No console output on DietPi                        | Use `/var/lib/freelss/freelss.log` (not the SSH terminal)                                               |


