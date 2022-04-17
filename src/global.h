/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <Arduino.h>
#include "tanklevel.h"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Preferences.h>
#define SPIFFS LITTLEFS 
#include <LITTLEFS.h>
#include "MQTTclient.h"
#include "wifimanager.h"

#define GPIO_HX711_DOUT 33                  // GPIO pin to use for DT or DOUT
#define GPIO_HX711_SCK 32                   // GPIO pin to use for SCK
#define webserverPort 80                    // Start the Webserver on this port
#define NVS_NAMESPACE "tanksensor"          // Preferences.h namespace to store settings

RTC_DATA_ATTR struct timing_t {
  // Check Services like MQTT, ...
  uint64_t lastServiceCheck = 0;               // last millis() from ServiceCheck
  const unsigned int serviceInterval = 30000;  // Interval in ms to execute code

  // Sensor data in loop()
  uint64_t lastSensorRead = 0;              // last millis() from Sensor read
  const unsigned int sensorInterval = 5000; // Interval in ms to execute code

  // Setup executing in loop()
  uint64_t lastSetupRead = 0;               // last millis() from Setup run
  const unsigned int setupInterval = 15 * 60 * 1000 / 255;   // Interval in ms to execute code
} Timing;

RTC_DATA_ATTR uint64_t sleepTime = 0;       // Time that the esp32 slept

TANKLEVEL Tanklevel;
WIFIMANAGER WifiManager;
bool enableWifi = true;                     // Enable Wifi, disable to reduce power consumtion, stored in NVS

struct Button {
  const gpio_num_t PIN;
  bool pressed;
};
Button button1 = {GPIO_NUM_4, false};       // Run the setup (use a RTC GPIO)

String hostName;
uint32_t blePin;
DNSServer dnsServer;
AsyncWebServer webServer(webserverPort);
AsyncEventSource events("/events");
Preferences preferences;

MQTTclient Mqtt;
