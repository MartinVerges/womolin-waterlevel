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

#define GPIO_HX711_DOUT 33                  // GPIO pin to use for DT or DOUT
#define GPIO_HX711_SCK 32                   // GPIO pin to use for SCK
#define webserverPort 80                    // Start the Webserver on this port
#define NVS_NAMESPACE "tanksensor"          // Preferences.h namespace to store settings

RTC_DATA_ATTR struct timing_t {
  // Wifi interval in loop()
  uint64_t lastWifiCheck = 0;               // last millis() from WifiCheck
  const unsigned int wifiInterval = 30000;  // Interval in ms to execute code

  // Sensor data in loop()
  uint64_t lastSensorRead = 0;              // last millis() from Sensor read
  const unsigned int sensorInterval = 1000; // Interval in ms to execute code

  // Setup executing in loop()
  uint64_t lastSetupRead = 0;               // last millis() from Setup run
  const unsigned int setupInterval = 15 * 60 * 1000 / 255;   // Interval in ms to execute code
} Timing;

RTC_DATA_ATTR bool startWifiConfigPortal = false; // Start the config portal on setup() (default set by wakeup funct.)
RTC_DATA_ATTR uint64_t sleepTime = 0;             // Time that the esp32 slept

TANKLEVEL Tanklevel;

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


String getMacFromBT(String spacer = "") {
  String output = "";
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  for (int i = 0; i < 6; i++) {
    char m[3];
    sprintf(m, "%02X", mac[i]);
    output += m;
    if (i < 5) output += spacer;
  }
  return output;
}
