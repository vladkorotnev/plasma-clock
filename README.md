# PIS-OS

Personal Information System OS (formerly Plasma Information Screen OS).
(Not DOS, there is no disk in it! yet.)

A somewhat portable relatively-stylish pixel-art clock/weather station.

## System Requirements

The basic configuration without any bluetooth functionality (no Switchbot or Balance Board integration) seems to work just fine on an ESP32 WROOM. However to be less limited by RAM size in further features I've decided to make WROVER the requirement, so further versions are not guaranteed to run on WROOM.

## Predefined target devices

* `DEVICE_PLASMA_CLOCK`: a [clock](https://youtu.be/D4MiHmhhjeQ) that I built around a plasma screen from an old Japanese bus/train.
* `DEVICE_MICROPISOS`: a portable devkit for PIS-OS, using a 100x16 OLED from WinStar.

## Supported hardware and feature-flags

### Display (at least one required)

* Morio Denki 16101DS (see [below](#morio-denki-plasma-display-info), [driver](src/display//md_plasma.cpp), feature flag `HAS_OUTPUT_MD_PLASMA`)
* Winstar WEG010016A in 8-bit parallel mode ([driver](src/display/ws0010.cpp), feature flag `HAS_OUTPUT_WS0010`)

### Speaker (at least one required)

* Piezo speaker ([driver](src/sound/beeper.cpp), [music](src/sound/melodies.cpp))

### Haptics (WIP)

* Taptic Engine via 2N3904 transistor as an amp (WIP: wired in parallel with the speaker)

### Software flags

* `HAS_WORDNIK_API`: compile with the Word Of The Day service using [Wordnik API](https://developer.wordnik.com/). This requires SSL, so bloats the firmware size significantly.
* `HAS_BLUETOOTH_LE`: automatically set on ESP32. Required for Switchbot-over-BLE. Uses Arduino-BLE and increases firmware size significantly.
* `HAS_OTAFVU`: OTA updates via ArduinoOTA. Currently disabled due to partition size constraints from the above.

### Thermal sensors

* AM2322 over IIC ([driver](src/sensor/am2322.cpp), feature flag `HAS_TEMP_SENSOR`)
* Switchbot Meter over BLE (unstable, [driver](src/sensor/switchbot/api.cpp), feature flag `SWITCHBOT_METER_INTEGRATION`, needs extra ram of a WROVER module)

### Motion sensors

* Any which provides logic H when motion found, logic L when not found ([driver](src/sensor/motion.cpp), feature flag `HAS_MOTION_SENSOR`)

### Light sensors

* Opto-resistor in voltage divider mode ([driver](src/sensor/light.cpp), feature flag `HAS_LIGHT_SENSOR`)

### HID

* Keypad/D-Pad. Set feature flag `HAS_KEYPAD` and define `const keypad_definition_t HWCONF_KEYPAD` in the device definition. [Driver](src/input/keypad.cpp)
* Touch plane. E.g. a faceplate with touch sensitive arrow keys to work in place of a D-pad. Set feature flag `HAS_TOUCH_PLANE` and define `const touch_plane_definition_t HWCONF_TOUCH_PLANE` in the device definition. [Driver](src/input/touch_plane.cpp)

### Others

* Wii Balance Board. Set feature flag `HAS_BALANCE_BOARD_INTEGRATION`. [Driver](src/service/balance_board.cpp), based upon [code by Sasaki Takeru](https://github.com/takeru/Wiimote/tree/d81319c62ac5931da868cc289386a6d4880a4b15), requires WROVER module

## Morio Denki Plasma Display Info

**This display uses high voltage, which could be lethal!!**

The display comes from a bus or a train, supposedly. 

It has the following labels on the PCBs:

* Morio Denki 6M06056 (the 8085-based control board I wasn't able to get running)
* MD 16101DS-CONT82 06 (the actual framebuffer/drive board)
* MD-24T-ADT (2) 8201 (the boards on the plasma tube itself)

Despite using a standard "HDD" Molex 4-pin connector for the drive board power, it expects +160V on the pin where normally +12V would be supplied. Take care not to mix up the power supplies. (Plugging in +12V into the plasma board doesn't seem to damage it. Plugging in +160V into an HDD, on the other hand...)

More detailed info is available in the following articles:

* На русском: https://habr.com/ru/companies/timeweb/articles/808805/
* 日本語で: https://elchika.com/article/b9f39c29-64aa-42ab-8f73-e6e27a72bd0e/
* Demo video: https://youtu.be/D4MiHmhhjeQ

You can also read the quest I went through trying to get it to run "in real time" at [EEVBlog Forums](https://www.eevblog.com/forum/repair/trying-to-figure-out-if-a-vfd-displaydriver-is-broken-(74-series-logic)/).