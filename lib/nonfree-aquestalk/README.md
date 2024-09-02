# AquesTalk ESP32 support for PIS-OS instructions

1. Purchase AquesTalk-ESP32 from the [official website](https://www.a-quest.com/products/aquestalk_esp32.html)
2. Put `libaquestalk.a` and `aquestalk.h` into this directory
3. Enable `HAS_AQUESTALK` in `device_config.h` of PIS-OS
4. Put your license key into `.env` and/or in PIS-OS WebAdmin. Otherwise some syllables will not be pronounced properly.
5. ゆっくりしていってね

## Why not flite or %OSSName%?

Do I look like someone who has 2+ megabytes to spare? They are not ゆっくり enough either.

But if you want to add support for multiple TTS engines, a PR is very much appreciated!
