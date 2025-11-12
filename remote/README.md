# Remote

### Hardware

Due to my poor 3D modelling skills I wanted to keep to the original remote casing as much as possible. Luckily I was able to cut out the top 
right hand part of the PCB in order to fit the vibration sensor snugly within the case. Wiring it up is easy enough:

```
Ground  ->  Top left pin of the original power button 
Power   ->  Run a small wire to the 3.3V pin on the ESP board
Signal  ->  Bottom left pin of the original power button 
```

The linked 3D printed case removes the cutout for the power button. If you'd rather not use a vibration sensor and are happy to continue using
the power button for wake you can just skip this part.

### Software

> [!IMPORTANT]
> You'll need to power on the ESP-NOW hub D1 first to get the MAC address from the logs. I'd suggest building the hub first.

For the software all you need to do here is change the `hubAddress` to the MAC address of your ESP-NOW D1 hub and flash this to the remote. 

This needs to be in hex format so:

```
uint8_t macAddress[] = {0xB4, 0xE6, 0x2D, 0x1F, 0x3D, 0x6D};
```

represents the same MAC address as:

```
B4:E6:2D:1F:3D:6D
```

It should be clear from the logs if this is all working, you'll be able to see if ESP-NOW successfully connected and if buttons are being registered correctly.