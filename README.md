# esp-alarm-assistant
Makes an alarm controllable in Home Assistant by using relays to press remote control buttons, and a microphone to determine whether the siren is ringing.

## Features
- Arm and disarm your alarm remotely
- Get notifications if your alarm's siren is ringing (if the minimum sound level measured during a certain amount of time is above a certain threshold
- Cause your alarm's siren to ring if your alarm supports it ("panic mode")
- ... get metrics and act on the ambiant sound level ?

## Preview

# Prerequisites

## Hardware
- An alarm with a siren and an on/off remote control
- An [M5StickC Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit) or other ESP32. You can get one [here](https://www.digikey.fr/fr/products/detail/m5stack-technology-co-ltd/K016-H/15771301?utm_medium=email&utm_source=oce&utm_campaign=3314_OCE22RT&utm_content=productdetail_FR&utm_cid=2455007&so=77654399&mkt_tok=MDI4LVNYSy01MDcAAAGGssdqOVJqwROm0PU1SuWvYCzapjFBFw1Lb8uv0_POoFERIHCwqu1031kKeph3oC28WtCSvfbDsEaWA2LanEIs7nHUU1Lz_iwg8LU2JH1x) in France for example. If you get another ESP32, you will need an external microphone.
- A pair of relays, or optocouplers, or other means to trigger the alarm remote control buttons. I used [this kit](https://shop.m5stack.com/products/2-channel-spst-relay-unit) for minimal hardware tinkering

## Software
- [The Arduino IDE](https://www.arduino.cc/en/software). Follow [this tutorial](https://docs.m5stack.com/en/arduino/arduino_development) for setting up the Aruino IDE for use with the M5StickC Plus.
- Home Assistant
- An MQTT Broker, such as the "Mosquitto broker" add-on for Home Assistant

# Getting started

## Hardware

## Software
