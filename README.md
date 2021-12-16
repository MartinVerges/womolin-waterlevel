# rv-smart-tanksensor

[![GitHub Super-Linter](https://github.com/MartinVerges/rv-smart-tanksensor/workflows/Lint%20Code%20Base/badge.svg)](https://github.com/marketplace/actions/super-linter)

DIY project to build a smart fuel sensor for RVs or in other projects. It is based on air pressure and is able to measure fresh water as well as grey and black water levels with high precision.

This project is still in development, it's a rough prototype and I'm happy about pull requests to add more functionality, improve existing ones or just feedback.

## Why?

After I have long found on the market for camping nothing that is on the one hand affordable and on the other hand also technically good, I have started the development of my own tank sensor.

Thanks to the ESP32 this can be queried in the future directly via WLAN or Bluetooth (work in progress). 

## Focus of this Project

The following aspects are the focus of this project:

 * Easy to use and rebuild
 * Precise in the evaluation of the data
 * Stand alone usable
 * Usable in all liquids

## Show me how it looks

#### Screenshots from the mobile UI
<img src="images/level.jpg?raw=true" alt="Live level view" width="20%"><img src="images/setup-linear.jpg?raw=true" alt="Setup for linear tanks" width="20%"><img src="images/setup-dynamic.jpg?raw=true" alt="Setup for dynamic tanks" width="20%"><img src="images/ota.jpg?raw=true" alt="Integrated over the air update" width="20%">

#### Photos of the prototype
<img src="images/prototype1.jpg?raw=true" alt="Prototype Pictures" width="30%"><img src="images/prototype2.jpg?raw=true" alt="Prototype Pictures" width="30%"><img src="images/prototype3.jpg?raw=true" alt="Prototype Pictures" width="30%">

## BOM - Bill of Materials

To build this sensor yourself, you need:

 * 1x ESP32 ESP32-WROOM-32 (between 1.75€ and 8.00€)
 * 1x MPX53DP Pressure Sensor (~ 8.00€)
 * 1x HX711 24-bit ADC (~2.50€)
 * 1x Pushbutton (~0.15€)
 * 1x Tube with 4mm inner width (~1€) 
 
 In addition you need some few small cables and soldering equipment to build the circuit.


## How to build this PlatformIO based project

1. [Install PlatformIO Core](http://docs.platformio.org/page/core.html)
2. Run these commands:

```
    # Change directory into the code folder
    > cd rv-smart-tanksensor

    # Build project
    > platformio run

    # Upload firmware
    > platformio run --target upload
```

## Alternatives

### Pressure based (like this project)

The only know alternative SuperSense tank sensor is available at around 259€ for purchase, but the price is extremely high and the data cannot be accessed digitally as it's proprietary.

### Resistance based

There are a lot of cheapest probes which all show only rough and mostly wrong data. These are mostly based on measuring the resistance or continuity between two or more conductive rods that come into contact with water.

These probes usually cost more than the components in this project, but are extremely inaccurate, prone to calcification and malfunction.

# License

This project and all of it's deliverables are free to use for every private person in his own private projects.
If you wan to use it in comercial projects, you have to ask for permission!
