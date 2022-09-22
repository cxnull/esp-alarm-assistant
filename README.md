# esp-alarm-assistant
**esp-alarm-assistant** is a solution for making an alarm controllable in [Home Assistant](https://www.home-assistant.io/) by using relays to press remote control buttons, and a microphone to determine whether the siren is ringing.

This project was designed for an [M5StickC Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit), and has been tested with a [Somfy Protexiom](https://boutique.somfy.fr/alarme-securite/somfy-protect/alarmes-sans-fil.html) alarm and [this remote control](https://boutique.somfy.fr/telecommande-alarme-on-off-groupes.html).

## Features
- Arm and disarm your alarm remotely or via automations thanks to Home Assistant (via geolocation for example).
- Get notifications if your alarm's siren is ringing (if the minimum sound level measured during a certain amount of time is above a certain threshold).
- Cause your alarm's siren to ring if your alarm supports it ("panic mode"), for example if your [Frigate](https://frigate.video/) instance has detected a person.
- Pressing the "M5" button next to the screen turns the screen on or off, for a live display of the sound level (and for checking that esp-alarm-assistant is running correctly).
- ... get metrics and act on the ambiant sound level, for rudimentary presence detection?

## Preview
![Preview](doc/preview.jpg)

# Prerequisites

## Hardware
- An alarm with a siren and an on/off remote control.
- An [M5StickC Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit) or other ESP32. You can get one [here](https://www.digikey.fr/fr/products/detail/m5stack-technology-co-ltd/K016-H/15771301?utm_medium=email&utm_source=oce&utm_campaign=3314_OCE22RT&utm_content=productdetail_FR&utm_cid=2455007&so=77654399&mkt_tok=MDI4LVNYSy01MDcAAAGGssdqOVJqwROm0PU1SuWvYCzapjFBFw1Lb8uv0_POoFERIHCwqu1031kKeph3oC28WtCSvfbDsEaWA2LanEIs7nHUU1Lz_iwg8LU2JH1x) in France for example. If you use anything else, you might need an external microphone and will likely have to make changes to the source code.
- A pair of relays, or optocouplers, or other means to trigger the alarm remote control buttons. I used [this kit](https://shop.m5stack.com/products/2-channel-spst-relay-unit) for minimal soldering.

## Software
- [Home Assistant.](https://www.home-assistant.io/)
- [The Arduino IDE](https://www.arduino.cc/en/software). Follow [these instructions](https://docs.m5stack.com/en/arduino/arduino_development) for setting up the Aruino IDE for use with the M5StickC Plus.
- An MQTT broker, such as the "Mosquitto broker" add-on for Home Assistant.

# Getting started

## Hardware
- On your alarm's remote control buttons, identify which button pins you need to short in order to trigger the arming and disarming. On [this Somfy remote control](https://boutique.somfy.fr/telecommande-alarme-on-off-groupes.html), the two connectors on one side of each button can be used (e.g. the two left pins of each button, or the two right pins of each button).
- Solder leads to those pins on one side, and connect them to the relays on the other side.
- Once the code has been uploaded and tested on the M5StickC PLUS, install the assembly close to the alarm siren, but in a place where it can't be seen or found easily.

## Software

### Connecting your M5StickC Plus to Home Assistant
- Edit [esp-alarm-assistant.ino](src/esp-alarm-assistant.ino) in order to configure your M5StickC Plus:

``` c
// Configuration
const char* ssid            = "Wi-Fi SSID";                                 // Wi-Fi SSID
const char* password        = "Wi-Fi password";                             // Wi-Fi password
const char* mqtt_server     = "MQTT broker IP address";                     // MQTT broker IP address
const uint16_t mqtt_port    = 12345;                                        // MQTT broker port
const char* mqtt_login      = "MQTT broker username";                       // MQTT broker username
const char* mqtt_password   = "MQTT broker password";                       // MQTT broker password
const char* mqtt_topic      = "MQTT topic for controlling the alarm";       // MQTT topic for controlling the alarm
const char* mqtt_topic_db   = "MQTT topic for reporting noise level";       // MQTT topic for reporting noise level
const char* arm_message     = "MQTT payload for arming the alarm";          // MQTT payload for arming the alarm
const char* disarm_message  = "MQTT payload for disarming the alarm";       // MQTT payload for disarming the alarm
const char* sos_message     = "MQTT payload for sounding the alarm";        // MQTT payload for sounding the alarm
const uint16_t sos_duration = 16000;                                        // Milliseconds on the "ON" button for sounding the alarm
const uint16_t send_every   = 5000;                                         // Milliseconds between sound level reports
const uint16_t time_between_samples = 100;                                  // Milliseconds between each sound measurement
```
- Once the code is uploaded on the M5StickC Plus, you should:
  - See it on your network.
  - Start seeing MQTT messages coming in with sound level reports. If you're using the Home Assistant "Mosquitto broker" add-on, go to Settings --> Devices & Services --> On the "Mosquitto broker" add-on card, click on "CONFIGURE" --> In the "Listen to a topic" card, enter what you entered for "MQTT topic for reporting noise level" and click on "START LISTENING". You should see these kinds of messages come in at the frequency you set for `send_every`.
  
 ``` json
 {
    "min": 33.1,
    "avg": 37.92,
    "max": 48.85,
    "N": 50
}
```
  - `min` is the minimum sound level value measured during the `send_every` period.
  - `avg` is the average sound level value measured during the `send_every` period.
  - `max` is the maximum sound level value measured during the `send_every` period.
  - `N`is the number of sound measurements made during the `send_every` period. This value depends on the value for `time_between_samples`.

### Creating an Alarm Control Panel
Enter the following in your `configuration.yaml` file:
``` yaml
alarm_control_panel:
  - platform: manual
    name: Smart Alarm
    code: !secret alarm_code
    code_arm_required: false
    disarm_after_trigger: false
    delay_time: 0
    arming_time: 0
    trigger_time: 0
```

And the following in your `secrets.yaml` file:
``` yaml
alarm_code: 1234
```

then restart Home Assistant.

In Lovelace you can then create an `Alarm Panel` card for arming and disarming your alarm.

Documentation: https://www.home-assistant.io/integrations/manual/

### Creating sound level sensors
Enter the following in your `configuration.yaml` file:

``` yaml
mqtt:
  sensor:
    - name: "Alarm min dB"
      state_topic: ""MQTT topic for reporting noise level""
      unit_of_measurement: "dB"
      value_template: "{{ value_json.min }}"
    - name: "Alarm avg dB"
      state_topic: ""MQTT topic for reporting noise level""
      unit_of_measurement: "dB"
      value_template: "{{ value_json.avg }}"
    - name: "Alarm max dB"
      state_topic: ""MQTT topic for reporting noise level""
      unit_of_measurement: "dB"
      value_template: "{{ value_json.max }}"
```

then restart Home Assistant.

Note that only the first sensor ("Alarm min dB") is actually required, but the two others can be nice to have in order to have information about your environment (e.g. for presence detection or other purposes).

### Automation for sending the arming message
This automation will be triggered by the alarm control panel.

``` yaml
alias: 🚨 Send the arming message
description: ""
trigger:
  - platform: state
    entity_id:
      - alarm_control_panel.smart_alarm
    to: armed_away
condition: []
action:
  - service: mqtt.publish
    data:
      topic: "MQTT topic for controlling the alarm"
      payload: "MQTT payload for arming the alarm"
mode: single
```

### Automation for sending the disarming message
This automation will be triggered by the alarm control panel if the code that was entered matches `alarm_code` that was defined in `secrets.yaml`.

``` yaml
alias: 🚨 Send the disarming message
description: ""
trigger:
  - platform: state
    entity_id:
      - alarm_control_panel.smart_alarm
    to: disarmed
condition: []
action:
  - service: mqtt.publish
    data:
      topic: "MQTT topic for controlling the alarm"
      payload: "MQTT payload for disarming the alarm"
mode: single
```

### Automation for notifying that the siren is ringing
This automation sends a notification if the *minimum* sound level is above 80 dB for more than 6 seconds. The notification contains a link to the Lovelace Alarm Control Panel, in this case `/lovelace-test/alarm`. This notification is tailored for use with the Android Companion App. If you're not using Android, you can [check the documentation](https://companion.home-assistant.io/docs/notifications/critical-notifications/) for tuning the notifications accordingly.

``` yaml
alias: 🚨 Alarm siren is ringing
description: ""
trigger:
  - platform: numeric_state
    entity_id: sensor.db_min_alarme
    for:
      hours: 0
      minutes: 0
      seconds: 6
    above: 80
condition: []
action:
  - service: notify.mobile_app
    data:
      title: 🚨 Alarm siren is ringing
      message: The minimum sound level was above 80 dB for more than 6 seconds
      data:
        actions:
          - action: URI
            title: Check it out
            uri: /lovelace-test/alarm
        ttl: 0
        priority: high
        color: red
        notification_icon: mdi:alarm-light
        push:
          sound:
            name: default
            volume: 1
  - service: alarm_control_panel.alarm_trigger
    data: {}
    target:
      entity_id: alarm_control_panel.smart_alarm
mode: single
```

### Automation for indicating that esp-alarm-assistant is offline
If there is no change in `sensor.db_min_alarme` for more than one minute, then it's safe to say that esp-alarm-assistant is offline.
``` yaml
alias: 🚨 esp-alarm-assistant is offline
description: ""
trigger:
  - platform: state
    entity_id:
      - sensor.db_min_alarme
    for:
      hours: 0
      minutes: 1
      seconds: 0
condition: []
action:
  - service: notify.mobile_app
    data:
      title: 🚨 esp-alarm-assistant is offline
      message: There was no change in sensor.db_min_alarme for more than one minute
      data:
        actions:
          - action: URI
            title: Check it out
            uri: /lovelace-test/alarm
        ttl: 0
        priority: high
        color: red
        notification_icon: mdi:alarm-light
        push:
          sound:
            name: default
            volume: 1
mode: single
```

### Script for sounding the alarm
This script can be used by automations or buttons for sounding the alarm by triggering "panic mode" if supported.

``` yaml
alias: 🚨 Sound the alarm
sequence:
  - service: mqtt.publish
    data:
      topic: "MQTT topic for controlling the alarm"
      payload: "MQTT payload for sounding the alarm"
mode: single
icon: mdi:alarm-light
```

### Warning
The automations created above contain secrets that you might want to hide. If that is the case, there is a [non-trivial way to do this](https://community.home-assistant.io/t/how-to-use-secrets-in-automations-yaml/312824/3?u=cxnull).

# Acknowledgements
- The whole sound level detection part was taken from [this awesome project](https://qiita.com/tomoto335/items/263b23d9ba156de12857) by [@tomoto335](https://twitter.com/tomoto335). The original source code is available [here](https://gist.githubusercontent.com/tomoto/6a1b67d9e963f9932a43c984171d80fb/raw/4c27b16745debfc93d39006bb03307d3958a3b28/LoudnessMeter.ino).
- The [M5StickC Plus documentation](https://github.com/m5stack/M5StickC-Plus).

# License
**esp-alarm-assistant** is licensed under the [MIT License](LICENSE).
