/**
 * @file global.h
 * @author Martin Verges <martin@veges.cc>
 * @version 0.1
 * @date 2022-07-09
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/waterlevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#include <Arduino.h>
#include "tanklevel.h"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include "MQTTclient.h"
#include "wifimanager.h"

#define webserverPort 80                    // Start the Webserver on this port
#define NVS_NAMESPACE "tanksensor"          // Preferences.h namespace to store settings

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 myBMP;

RTC_DATA_ATTR struct timing_t {
  // Check Services like MQTT, ...
  uint64_t lastServiceCheck = 0;               // last millis() from ServiceCheck
  const unsigned int serviceInterval = 30000;  // Interval in ms to execute code

  // Sensor data in loop()
  uint64_t lastSensorRead = 0;                 // last millis() from Sensor read
  const unsigned int sensorInterval = 500;     // Interval in ms to execute code

  // Setup executing in loop()
  uint64_t lastSetupRead = 0;                  // last millis() from Setup run
  const unsigned int setupInterval = 15 * 60 * 1000 / 255;   // Interval in ms to execute code
} Timing;

RTC_DATA_ATTR uint64_t sleepTime = 0;       // Time that the esp32 slept

WIFIMANAGER WifiManager;
bool enableWifi = true;                     // Enable Wifi, disable to reduce power consumtion, stored in NVS

TANKLEVEL LevelManager1(GPIO_NUM_33, GPIO_NUM_32);
#define LEVELMANAGERS 1
TANKLEVEL * LevelManagers[LEVELMANAGERS] = {
  &LevelManager1
};

struct Pump {
  const gpio_num_t PIN;
  bool enabled;
  uint64_t duration;
  uint64_t starttime;
};
Pump airPump = {GPIO_NUM_19, false, 5000, 0};   // Airpump using GPIO to repressure the tube

struct Button {
  const gpio_num_t PIN;
  bool pressed;
};
Button button1 = {GPIO_NUM_4, false};       // Run the setup (use a RTC GPIO)

String hostName;
uint32_t blePin;
DNSServer dnsServer;
AsyncWebServer webServer(webserverPort);
Preferences preferences;

MQTTclient Mqtt;

uint64_t runtime() {
  return rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) / 1000;
}
