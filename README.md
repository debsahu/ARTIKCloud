# ARTIKCloud

## ESP8266 Connecting to Samsung ARTIK Cloud https://my.artik.cloud/

[![Build Status](https://travis-ci.com/debsahu/ARTIKCloud.svg?branch=master)](https://travis-ci.com/debsahu/ARTIKCloud) [![License: MIT](https://img.shields.io/github/license/debsahu/ARTIKCloud.svg)](https://opensource.org/licenses/MIT) [![version](https://img.shields.io/github/release/debsahu/ARTIKCloud.svg)](https://github.com/debsahu/ARTIKCloud/releases/tag/1.0.0) [![LastCommit](https://img.shields.io/github/last-commit/debsahu/ARTIKCloud.svg?style=social)](https://github.com/debsahu/ARTIKCloud/commits/master)

An application made for ESP8266 to comunicate with ARTIK Cloud
- Connects to ARTIK Cloud via MQTT SSL port 8883
- ARTIK Cloud is connected to all assistant platforms, integration is seamless
- Compiles on ESP8266-arduino core v2.4.2, v2.5.0-beta3, git version

## YouTube

[![ARTIKCloud](https://img.youtube.com/vi/xxxxxxxx/0.jpg)](https://www.youtube.com/watch?v=xxxxxxxx)

## ARTIK Cloud Instructions

[Follow instructions](https://github.com/debsahu/ARTIKCloud/tree/master/doc/README.md)

## Libraries Needed

[platformio.ini](https://github.com/debsahu/ARTIKCloud/blob/master/platformio.ini) is included, use [PlatformIO](https://platformio.org/platformio-ide) and it will take care of installing the following libraries.

| Library                   | Link                                                       |                                            |
|---------------------------|------------------------------------------------------------|--------------------------------------------|
|ESPAsyncUDP                |https://github.com/me-no-dev/ESPAsyncUDP                    |                                            |
|ESPAsyncTCP                |https://github.com/me-no-dev/ESPAsyncTCP                    |                                            |
|Adafruit NeoPixel          |https://github.com/adafruit/Adafruit_NeoPixel               |                                            |
|NeoPixelBus                |https://github.com/Makuna/NeoPixelBus                       | optional: `#define USE_NEOPIXELBUS`        |
|ESPAsyncWiFiManager        |https://github.com/alanswx/ESPAsyncWiFiManager              | `#define USE_WIFIMANAGER`                  |
|ESPAsyncDNSServer          |https://github.com/devyte/ESPAsyncDNSServer                 |                                            |
|ESP Async WebServer        |https://github.com/me-no-dev/ESPAsyncWebServer              | `#define USE_ASYNC_WEBSERVER`               |
|ArduinoJson                |https://github.com/bblanchon/ArduinoJson                    |                                            |
|WebSockets                 |https://github.com/Links2004/arduinoWebSockets              | optional: `//#define USE_ASYNC_WEBSERVER`   |
|WiFiManager                |https://github.com/tzapu/WiFiManager                        | optional: `//#define USE_ASYNC_WEBSERVER` & `#define USE_WIFIMANAGER` |

## ESP32

Check this [issue](https://github.com/debsahu/ARTIKCloud/issues/1) for latest progress.

# Hardware

- Connect 5V to + of WS2812B and VIN of NodeMCU or voltage regulator in of your favorite ESP8266 board
- Connect GND to GND of WS2812B and ESP device
- Connect GPIO3/RX pin of ESP8266 to WS2812B
- Attach at least a 1000μF capacitor between 5V and GND near the first LED

![hardware](https://github.com/debsahu/ARTIKCloud/blob/master/doc/hardware.png)