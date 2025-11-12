## Intro

Many thanks to The Stock Pot for providing the inspiration for this project. The vast majority of the groundwork was done in his excellent [Everything Remote project.](https://www.printables.com/model/1281626-everything-remote-esp32-powered-universal-remote)

After building the initial version, there were a few aspects I couldn’t quite get comfortable with—specifically, needing to wake the device via the power button and waiting a couple of seconds for Wi-Fi to connect. Whilst not as polished as the OG blog post, with the changes outlined below, I believe I’ve managed to mostly resolve those issues.

## Lift to wake

The most obvious improvement was to introduce an accelerometer to wake the remote when lifted. However, all accelerometers I could find require additional data pins beyond the standard interrupt, ground, and power connections in order to perform an initial calibration. Once power is lost, that calibration data is erased, leaving the sensor overly sensitive until recalibrated. Since I didn’t want to sacrifice several button pins for this, it wasn’t a practical option.

I experimented with a basic tilt sensor (KY-020), but it proved too insensitive, requiring exaderated movements to trigger. I eventually settled on a simple SW-18015P vibration sensor, repurposing the power button pin. Ideally, this component would be integrated into the PCB, but it’s small enough to fit neatly into the top section of the case with only minor board modifications.

This setup performs well: in most cases, a natural “remote-lifting” motion wakes the device instantly, registering immediate button presses. After a configurable timeout, the remote returns to sleep mode. If the lift doesn’t trigger, a gentle shake reliably does. The relatively short timeout also helps conserve battery life if the remote is jostled around on the sofa, though the sensor requires a fairly distinct motion to activate.

<p align="center">
  <img src="https://github.com/user-attachments/assets/50f4b0ec-de1b-47e9-a910-e0d7976037ac" alt="Remote showing vibration sensor in top right corner" width="350"/>
  <img src="https://github.com/user-attachments/assets/0c32f2b1-6a84-488d-a1ad-759b4167129c" alt="Close up of vibration sensor" width="350"/>
</p>

## Central hub

Instead of running ESPHome directly on the remote (which requires a full Wi-Fi connection each time it wakes), the remote now uses a lightweight ESP-NOW program that communicates with a central hub, which then forwards commands to Home Assistant.

The hub consists of two D1 Mini boards and an IR receiver/blaster. One D1 handles ESP-NOW communication, while the other runs ESPHome. They communicate via UART. This dual-board design overcomes an ESP-NOW limitation that requires all devices to operate on the same fixed Wi-Fi channel. With this setup, one D1 remains fixed to the ESP-NOW channel used by the remote, while the other connects freely to my home Wi-Fi—avoiding any impact on network performance.

Thanks to the lift-to-wake functionality and ESP-NOW, remote responsiveness is effectively instantaneous, with the added bonus of improved battery efficiency. ESP-NOW is relatively new to me, but it eliminates much of the latency and power overhead of standard Wi-Fi, making it ideal for this application.

I did consider using BLE to simplify the hub setup, but nearly all my research pointed to ESP-NOW as the better choice. Since ESP-NOW transmissions are so lightweight, there was little benefit to placing the IR blaster on the remote itself; the trade-off is that IR control is now limited to the same room as the hub.

Looking ahead, I’d like to integrate a USB-C port into the hub, turning it into a combined charging dock and control center. With continuous power available, the hub can easily support additional sensors or modules as needed.

## Interested?

If you're interested in creating something similar for yourself I've uploaded the relevant code to this repo and the links to the amended print files below. I'm not hugely familiar with ESP-NOW and have done little with
ESPHome before so a lot of the code was generated with help from an LLM (with **a lot** of back and forth) but it's been working reliably for me for a few weeks now.

**Resources:**

More detailed readme files are in each directory.

- 3D models that remove the power button cutout and also provides a couple extra screw holes that are large enough for heat set inserts for a more rigid case.
- hub_a_espnow.ino: Flash this to the D1 running ESP-NOW, I use the Arduino IDE. Flash this first as you'll need the MAC address for the remote.
- hub_b_esphome.yaml: Flash this to the D1 running ESPHome, use the normal ESPHome builder.
- remote.ino: Flash this to the remote, I use the Arduino IDE.
