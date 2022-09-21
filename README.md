# esp-alarm-assistant
**esp-alarm-assistant** is a solution for making an alarm controllable in Home Assistant by using relays to press remote control buttons, and a microphone to determine whether the siren is ringing.

This project was designed for an M5StickC Plus, and has been tested with a [Somfy Protexiom](https://boutique.somfy.fr/alarme-securite/somfy-protect/alarmes-sans-fil.html) alarm and [this remote control](https://boutique.somfy.fr/telecommande-alarme-on-off-groupes.html).

## Features
- Arm and disarm your alarm remotely or via automations thanks to Home Assistant
- Get notifications if your alarm's siren is ringing (if the minimum sound level measured during a certain amount of time is above a certain threshold)
- Cause your alarm's siren to ring if your alarm supports it ("panic mode"), for example if your [Frigate](https://frigate.video/) instance has detected a person.
- ... get metrics and act on the ambiant sound level, for rudimentary presence detection?

## Preview

# Prerequisites

## Hardware
- An alarm with a siren and an on/off remote control
- An [M5StickC Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit) or other ESP32. You can get one [here](https://www.digikey.fr/fr/products/detail/m5stack-technology-co-ltd/K016-H/15771301?utm_medium=email&utm_source=oce&utm_campaign=3314_OCE22RT&utm_content=productdetail_FR&utm_cid=2455007&so=77654399&mkt_tok=MDI4LVNYSy01MDcAAAGGssdqOVJqwROm0PU1SuWvYCzapjFBFw1Lb8uv0_POoFERIHCwqu1031kKeph3oC28WtCSvfbDsEaWA2LanEIs7nHUU1Lz_iwg8LU2JH1x) in France for example. If you get another ESP32, you will need an external microphone and likely make changes to the source code.
- A pair of relays, or optocouplers, or other means to trigger the alarm remote control buttons. I used [this kit](https://shop.m5stack.com/products/2-channel-spst-relay-unit) for minimal soldering

## Software
- [The Arduino IDE](https://www.arduino.cc/en/software). Follow [these instructions](https://docs.m5stack.com/en/arduino/arduino_development) for setting up the Aruino IDE for use with the M5StickC Plus.
- Home Assistant
- An MQTT Broker, such as the "Mosquitto broker" add-on for Home Assistant

# Getting started

## Hardware
- 

## Software

# Acknowledgements
- The whole sound level detection part was taken from [this project](https://qiita.com/tomoto335/items/263b23d9ba156de12857) by [@tomoto335](https://twitter.com/tomoto335). The original source code is available [here](https://gist.githubusercontent.com/tomoto/6a1b67d9e963f9932a43c984171d80fb/raw/4c27b16745debfc93d39006bb03307d3958a3b28/LoudnessMeter.ino).
- The [M5StickC Plus documentation](https://github.com/m5stack/M5StickC-Plus)

# Disclaimer
The author takes no responsability
