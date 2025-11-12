# Central hub

### Hardware

You'll need: 
- 2 x D1 mini. I'm using the cheaper ESP8266 ones, you may need to change some libraries for ESP32 models
- Some wire, I generally use 22 awg solid core wire
- USB-C power breakout board. Any will do, we only need power and ground
- 2 x 1N5819 diodes. These will stop each board feeding power to the other when using direct USB access.


- Perfboard (optional). I find this easier than having a bundle of parts, it makes it easier to solder a power/ground bar from the breakout board too.
- IR blaster  PCB0100 (optional)
- IR receiver KY-022 (optional)

**Basic wiring:**

```
Hub A (ESP-NOW):

5V      -> 5V on the USB-C breakout. Use a diode between, cathode (band) on the D1 side
GND     -> GND on the USB-C breakout
TX      -> RX on hub B
RX      -> TX on hub B

Hub B (ESPHome):

5V      -> 5V on the USB-C breakout. Use a diode between, cathode (band) on the D1 side
GND     -> GND on the USB-C breakout
TX      -> RX on hub A
RX      -> TX on hub A

IR Blaster:

VCC     -> 5V on the USB-C breakout
GND     -> GND on the USB-C breakout
DAT     -> D2 on hub B. No need for anything additional here as the IR board contains all the components we need.

IR Receiver: 

VCC     -> 5V on the USB-C breakout
GND     -> GND on the USB-C breakout
SIG     -> D5 on hub B

```

### Software

#### Hub A (ESP-NOW)

Start by flashing the ESP-NOW sketch onto Hub A, you shouldn't need to change anything in this file. You should see 
"Hub A ready on channel... " in the serial logs once it's started up. You should also see the hubs MAC address, note this down
as you'll need it for the remote (you can flash the remote from anytime now but I'd get the hub fully setup first).

#### Hub B (ESPHome)

This esphome config should mostly be plug and play. I've split it out the IR into a section denoted by "start of IR" and "end of IR",
you can remove this if you don't want IR functionality. 

Once you have both hubs and the remote all flashed you should be wake up the remote and see button presses showing in the ESPHome logs,
once the device is configured in HomeAssistant you should also see the events there.

The IR blaster is configured for Samsung at the moment, you may want to change this to suit your needs.

### HomeAssistant

As per the original authors code you should see button press events arriving in HomeAssistant, you can confirm this by listening to the `esphome.remote_button_pressed`
event topic. To use this in an automation :

```
actions:
    - choose:
      - conditions:
          - condition: template
            value_template: >-
              {% set msg_json = trigger.event.data.message.split('=>')[-1] | trim %}
              {% set msg = msg_json | from_json %}
              {{ msg.button == 'volume_up' and msg.type == 'single' }}
        sequence:
          - alias: Vol up IR
            action: esphome.everything_remote_hub_send_samsung_ir
            metadata: {}
            data:
              data: "0xE0E0E01F"
              nbits: 32  
```

For the IR functionality you should see a "Last IR code" sensor attached to the device in HomeAssistant, point a remote at the receiver, press a button, and 
you should see the board light flash and this sensor change e.g. `Samsung data=0xE0E0E01F bits=32`

Once you have that code you can then blast it in your automation:

```
action: esphome.everything_remote_hub_send_samsung_ir
metadata: {}
data:
  data: "0xE0E0E01F"
  nbits: 32  
```