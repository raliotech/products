Ralio board based on ESP8266 core
===========================================

## Arduino on ESP8266

This project brings support for the ESP8266 chip to the Arduino environment. It lets you write sketches, using familiar Arduino functions and libraries, and run them directly on ESP8266, with no external microcontroller required.

ESP8266 Arduino core comes with libraries to communicate over WiFi using TCP and UDP, set up HTTP, mDNS, SSDP, and DNS servers, do OTA updates, use a file system in flash memory, and work with SD cards, servos, SPI and I2C peripherals.

## Contents
- Installing options:
  - [Using Boards Manager](#installing-with-boards-manager)
- [License and credits](#license-and-credits)   

### Installing with Boards Manager

Starting with 1.6.4, Arduino allows installation of third-party platform packages using Boards Manager. We have packages available for Windows, Mac OS, and Linux (32 and 64 bit).

- Install the current upstream Arduino IDE at the 1.8.9 level or later. The current version is on the [Arduino website](https://www.arduino.cc/en/software).
- Start Arduino and open the Preferences window.
- Enter ```https://arduino.esp8266.com/stable/package_esp8266com_index.json``` into the *File>Preferences>Additional Boards Manager URLs* field of the Arduino IDE. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install *esp8266* platform (and don't forget to select your ESP8266 board from Tools > Board menu after installation).

### License and credits ###

Arduino IDE is developed and maintained by the Arduino team. The IDE is licensed under GPL.

ESP8266 core includes an xtensa gcc toolchain, which is also under GPL.

Esptool.py was initially created by Fredrik Ahlberg (@themadinventor, @kongo), and is currently maintained by Angus Gratton (@projectgus) under GPL 2.0 license.

[Espressif's NONOS SDK](https://github.com/espressif/ESP8266_NONOS_SDK) included in this build is under Espressif MIT License.

ESP8266 core files are licensed under LGPL.

[SPI Flash File System (SPIFFS)](https://github.com/pellepl/spiffs) written by Peter Andersson is used in this project. It is distributed under the MIT license.

[umm_malloc](https://github.com/rhempel/umm_malloc) memory management library written by Ralph Hempel is used in this project. It is distributed under the MIT license.

[SoftwareSerial](https://github.com/plerup/espsoftwareserial) library and examples written by Peter Lerup. Distributed under LGPL 2.1.

[BearSSL](https://bearssl.org) library written by Thomas Pornin, built from https://github.com/earlephilhower/bearssl-esp8266, is used in this project.  It is distributed under the [MIT License](https://bearssl.org/#legal-details).

[LittleFS](https://github.com/ARMmbed/littlefs) library written by ARM Limited and released under the [BSD 3-clause license](https://github.com/ARMmbed/littlefs/blob/master/LICENSE.md).

[uzlib](https://github.com/pfalcon/uzlib) library written and (c) 2014-2018 Paul Sokolovsky, licensed under the ZLib license (https://www.zlib.net/zlib_license.html). uzlib is based on: tinf library by Joergen Ibsen (Deflate decompression); Deflate Static Huffman tree routines by Simon Tatham; LZ77 compressor by Paul Sokolovsky; with library integrated and maintained by Paul Sokolovsky.

Last but not the least, [ESP8266/Ardunio](https://github.com/esp8266/Arduino) has been a backbone behind this release.
