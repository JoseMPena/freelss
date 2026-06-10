# FreeLSS

FreeLSS is a laser scanning program for the Raspberry Pi. It allows a Raspberry Pi to function as the core to a complete turn table laser scanning system.

External hardware (turntable stepper, two laser modules and the lighting strip) is driven by an **Arduino Mega running the FreeLSS G-code firmware over USB**, not by the Pi's GPIO pins. The Pi only sends one G-code command per movement and waits for the firmware's `>` ready ack.

### COMPILE

These instructions assume you are running the latest version of Raspbian.  Other distros will likely require changes.

First, update the firmware to the latest version and reboot.
```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Install the dependencies that are managed by the package manager.
```
$ sudo apt-get install libpng-dev libjpeg-dev git-core gcc build-essential unzip sqlite3 libsqlite3-dev libmicrohttpd-dev libcurl4-openssl-dev libiw-dev libssl-dev
```

Download and build FreeLSS
```
$ git clone https://github.com/hairu/freelss
$ cd freelss
$ make
```

### Running FreeLSS
FreeLSS must be run as root (or as a user with permission to open the Arduino USB serial device — usually a member of the `dialout` group). The interface for FreeLSS is web based and by default runs on port 80.  When running, access it by navigating to http://localhost/ from the Raspberry Pi itself. Or access it from another machine on the network by the Raspberry Pi's IP or hostname.  For Example: http://raspberrypi/

On startup, FreeLSS opens the Arduino USB serial port (configured in the Setup page; empty = autodetect of `/dev/ttyACM*` and `/dev/ttyUSB*`) and sends `M100` as a liveness probe. The Setup page exposes a **Test Arduino Connection** button that re-runs the probe.

The following command starts FreeLSS.
```
$ cd src
$ sudo ./freelss
```

The following command runs the unit tests for the new G-code transport and adapters.
```
$ cd src
$ make test
```

The following command automatically starts FreeLSS everytime the Raspberry Pi is powered on.
```
$ make startup
```
