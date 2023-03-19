# womolin.tanklevel

DIY project to build a smart tank level sensor for RVs or in other projects.
It is based on air pressure and is able to measure fresh water as well as grey and black water levels with high precision.

This project is still in development, it's a working sensor and I'm happy about pull requests to add more functionality, improve existing ones or just feedback.

## Why?

After I have long found on the market for camping nothing that is on the one hand affordable and on the other hand also technically good, I have started the development of my own tank sensor.

Thanks to the ESP32 this can be queried directly via WLAN or Bluetooth.
In addition MQTT, the sensor value can be pushed into an MQTT broker.

## Focus of this Project

The following aspects are the focus of this project:

 * Easy to use and rebuild
 * Precise in the evaluation of the data
 * Stand alone usable
 * Usable in all type of liquids

## Show me how it looks

#### Video Demonstration
[![Live video demonstration](https://img.youtube.com/vi/gYvKPjnI3uc/0.jpg)](https://www.youtube.com/watch?v=gYvKPjnI3uc)

#### Screenshots from the mobile UI
<img src="images/status.png?raw=true" alt="Status" width="20%"><img src="images/setup.png?raw=true" alt="Calibration" width="20%"><img src="images/setup-uniform2.png?raw=true" alt="Uniform calibration" width="20%"><img src="images/config.png?raw=true" alt="Configuration" width="20%"><img src="images/wifi.png?raw=true" alt="Wifi Management" width="20%">

#### Photos of the prototype
<img src="images/prototype1.jpg?raw=true" alt="Prototype Pictures" width="30%"><img src="images/prototype2.jpg?raw=true" alt="Prototype Pictures" width="30%"><img src="images/mounting.jpg?raw=true" alt="Mounting" width="30%">

<img src="images/prototype-housing1.jpg?raw=true" alt="Prototype inside the housing" width="30%"><img src="images/prototype-housing2.jpg?raw=true" alt="Prototype inside the housing" width="30%">

#### Photos of the immersion tube

<img src="images/immersiontube1.jpg?raw=true" alt="immersion tube" width="30%"><img src="images/immersiontube2.jpg?raw=true" alt="immersion tube" width="30%"><img src="images/immersiontube3.jpg?raw=true" alt="immersion tube" width="30%">

#### Schematics
<img src="images/schematic.png?raw=true" alt="Schematic" width="40%">

#### Housing

The 3D printed housing can be found on [onshape as a editable document](https://cad.onshape.com/documents/dc5b401f0da730c8b1faabf2/w/d0c67204fcfba4efb6f8e658/e/1418b0722486737a7a11290c?renderMode=0&uiState=61bdca68d4b418569530fb02), or inside the folder [housing/*](housing/) as STL files.

## BOM - Bill of Materials

To build this sensor yourself, you need:

 * 1x [ESP32 ESP32-WROOM-32](https://womolin.de/produkt/wemos-d1-mini-esp32/)
 * 1x MPX53DP Pressure Sensor (~8.00€)
 * 1x [HX711 24-bit ADC](https://womolin.de/produkt/hx711-breakout-board/)
 * 1x Pushbutton (~0.15€)
 * 1x Tube with 20mm outher dimension (~1€) [HDPE 20mm](https://www.hornbach.de/shop/KWL-PE-HD-Rohr-20-mm-Laenge-2-m-Stange-12-5-bar/10193149/artikel.html)
 * 1x Tube with 4mm inner width (~1€) [PVC 4x1mm](https://www.hornbach.de/shop/Aquariumschlauch-Weich-PVC-Schlauch-4-x-1-mm-transparent-Meterware/5806466/artikel.html)
 * 1x Mini Air Pump (~2€) [3V 370](https://www.alibaba.com/product-detail/3-7V-6V-12VAquarium-for-fish_1600385811993.html)
 * 1x Small 12V to 5V power supply (~1€)
 * 1x PG screw fitting in the dimension for the 20mm tube
 * 1x Connector for the Tank itself, strongly depending on your individual situation
 
 In addition you need some few small cables and soldering equipment to build the circuit.

## Webinstaller

To make it super simple, our pre build firmware can be uploaded to your ESP32 using our Webinstaller.
You can find it at [https://webinstaller.womolin.de](https://webinstaller.womolin.de).

## OTA (Over-the-Air) Firmware files

If you want to update an already installed Sensor, you can upload the binarys directly to the sensor using the Web UI.
The latest version (current git main branch) is available at [https://s3.womolin.de/webinstaller/waterlevel-latest/firmware.bin](https://s3.womolin.de/webinstaller/waterlevel-latest/firmware.bin) and [https://s3.womolin.de/webinstaller/waterlevel-latest/littlefs.bin](https://s3.womolin.de/webinstaller/waterlevel-latest/littlefs.bin).

## How to build this PlatformIO based project

1. [Install PlatformIO Core](http://docs.platformio.org/page/core.html)
2. Run these commands:

```
    # Change directory into the code folder
    > cd womolin.tanklevel

    # Build project
    > platformio run -e esp32dev

    # Upload firmware
    > platformio run -e esp32dev --target upload
```

### Upload prebuild file

Here is a example command to upload existing firmware files.
```
/path/to/esptool.py --chip esp32 --port "/dev/ttyUSB0" --baud 921600 --before default_reset --after hard_reset \
    write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
    0x1000 bootloader_dio_80m.bin \
    0x8000 partitions.bin \
    0xe000 boot_app0.bin \
    0x10000 firmware.bin \
    0x190000 firmware.bin \
    0x310000 littlefs.bin
```
Please make sure that your ESP32 runs with these settings before uploading it.

## How to build the UI

As the UI requires a valid FontAweSome License, you can find a generated `littlefs.bin` with my subscription.
It's automatically uploaded to our S3 at [https://s3.womolin.de/webinstaller/gaslevel-latest/littlefs.bin](https://s3.womolin.de/webinstaller/gaslevel-latest/littlefs.bin).
Please feel free to take this one for your sensor.

### Build your own littlefs.bin

As I haven't found good icons with a free license, I choosed the pro version of fontawesome.
Therefore it's required to have a valid subscription in order to build the UI yourself.

Set your FontAweSome key:
```
npm config set "@fortawesome:registry" https://npm.fontawesome.com/
npm config set "//npm.fontawesome.com/:_authToken" XXXXXXXXXXXXXXXXXXXX
```

Build the UI:
```
cd ui
npm install
npm run build
cd ..
pio run -e esp32dev -t buildfs
```

## Operation

When the sensor is started for the first time, a WiFi configuration portal opens via which a connection to the central access point can be established.
As soon as the connection has been established, the sensor is available on the IP assigned by the DHCP and the hostname [tanklevel.local](http://tanklevel.local) provided by MDNS.
You can now log in to the webobverface and proceed with the sensor setup. 
To obtain a tank level, it is necessary to measure the tank.
This can be done in 2 different ways:

1) Uniformly shaped tanks whose water level rises evenly.
2) Unevenly shaped tanks that contain bulges, tapers or other complex shapes.

### Uniform

Install the Sensor in an empty Tank, navigate to the [setup](http://tanklevel.local/setup/).
Press the button to determine the lower value of the tank. The value then appears in the input field. 

### Unevenly

Install the Sensor in an empty Tank, navigate to the [setup](http://tanklevel.local/setup/).
Prepare everything to achieve a constant inflow into the tank.
The best way to do this is to use water connections to city networks with a larger cross-section.
Refueling through this water connection should be done within about 3 minutes.

Once you have prepared everything, start the setup and now turn on the water tap.
The sensor now determines the increase in water level and calculates the percentage distribution in the tank at the end of the process.
Once the tank is completely filled, turn off the water supply and wait about 5 seconds before completing the setup via the web interface so that the pressure can normalize.

## Android Bluetooth Low Energy (BLE) App

This sensor can be displayed using my [Android App](https://github.com/MartinVerges/smartsensors/). 

## Power saving mode

This sensor is equipped with various techniques to save power.
Unfortunately, the operation of WiFi causes relatively high power consumption (__45~55mA idle__, up to 250mA during transmission).
This is not a problem in RVs with larger batteries or solar panels, but may be too high in small installations.
Therefore, the WiFi module can be easily and conveniently switched off via the web interface, putting the sensor into a deep sleep mode.
Here, the consumption of the esp32 officially drops to about 10μA.
Including the used step down power supply it will still consume __12.2mA in this sleep mode__ with RTC ADC enabled, so around 1/4 of full featured wifi enabled consumtion.
At regular intervals, the sensor switches on briefly, checks the tank level and updates the analog output of the sensor.

Since the WiFi portal is not available in this mode, you can reactivate the WiFi by pressing the button on the device once.

## Wifi connection failed or unable to interact

The button on the device switches from Powersave to Wifi Mode.
When the button is pressed again, the sensor restarts and opens its own access point.
Here you can correct the WLAN data.

## Alternatives

### Pressure based (like this project)

The only know alternative SuperSense tank sensor is available at around 259€ for purchase, but the price is extremely high and the data cannot be accessed digitally as it's proprietary.

### Resistance based

There are a lot of cheapest probes which all show only rough and mostly wrong data.
These are mostly based on measuring the resistance or continuity between two or more conductive rods that come into contact with water.

These probes usually cost more than the components in this project, but are extremely inaccurate, prone to calcification and malfunction.

## Using it with Home Assistant

To use the data of the sensor in the best possible way from the MQTT in HomeAssistant, here is an example configuration.
Please put it into your `configuration.yaml` file.

```
mqtt:
  sensor:
    - name: "Freshwater Level"
      unique_id: "freshwaterlevel"
      state_topic: "freshwater/tanklevel1"
      unit_of_measurement: "%"
      icon: "mdi:water-percent"

    - name: "Freshwater sensor temperature"
      unique_id: "freshwatertemp"
      state_topic: "freshwater/temperature1"
      unit_of_measurement: "°C"
      icon: "mdi:temperature-celsius"

    - name: "Freshwater sensor air pressure"
      unique_id: "freshwaterpressure"
      state_topic: "freshwater/airPressure1"
      unit_of_measurement: "hPa"
      icon: "mdi:gauge"

```
Your dashboard configuration could look like that:

```
square: false
columns: 2
type: grid
cards:
  - type: vertical-stack
    cards:
      - type: gauge
        entity: sensor.freshwater_level
        severity:
          green: 50
          yellow: 20
          red: 0
        needle: false
        min: 0
        max: 100
      - type: entities
        entities:
          - entity: sensor.freshwater_sensor_temperature
            name: Temp.
          - entity: sensor.freshwater_sensor_air_pressure
            name: Airpres.
        show_header_toggle: false
```

# License

womolin.tanklevel (c) by Martin Verges.

This Project is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this work.
If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
