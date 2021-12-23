#include <Arduino.h>
#include "tanklevel.h"
#include <DNSServer.h>
#include <ESP_DoubleResetDetector.h>
#include <ESPAsyncWebServer.h>
#include <esp_bt.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}

const int GPIO_DAC_LEVEL_OUT = 25;          // Provide current level using a 1-3V level
const int GPIO_HX711_DOUT = 33;             // GPIO pin to use for DT or DOUT
const int GPIO_HX711_SCK = 32;              // GPIO pin to use for SCK
const int webserverPort = 80;               // Start the Webserver on this port
const String NVS_NAMESPACE = "tanksensor";  // Preferences.h namespace to store settings
const float DAC_MIN_MVOLT = 1000.0;         // DAC output minimum value (1V on 0% tank level)
const float DAC_MAX_MVOLT = 3000.0;         // DAC output maximum value (3V on 100% tank level)
const float DAC_VCC = 3300.0;               // DAC output maximum voltage from microcontroller 3.3V = 3300mV

RTC_DATA_ATTR struct timing_t {
  // Wifi interval in loop()
  uint64_t lastWifiCheck = 0;               // last millis() from WifiCheck
  const unsigned int wifiInterval = 30000;  // Interval in ms to execute code

  // Sensor data in loop()
  uint64_t lastSensorRead = 0;              // last millis() from Sensor read
  const unsigned int sensorInterval = 1000; // Interval in ms to execute code

  // Setup executing in loop()
  uint64_t lastSetupRead = 0;               // last millis() from Setup run
  const unsigned int setupInterval = 250;   // Interval in ms to execute code
} Timing;

bool enableWifi = false;                    // Enable Wifi, disable to reduce power consumtion, stored on NVS

RTC_DATA_ATTR bool startWifiConfigPortal = false;     // Start the config portal on setup()
RTC_DATA_ATTR uint64_t sleepTime;                     // Time the esp32 slept

TANKLEVEL Tanklevel;

struct Button {
  const gpio_num_t PIN;
  bool pressed;
};
RTC_DATA_ATTR Button button1 = {GPIO_NUM_4, false};   // Run the setup (use a RTC GPIO)

String hostName;
DNSServer dnsServer;
DoubleResetDetector* drd;
AsyncWebServer webServer(webserverPort);
AsyncEventSource events("/events");
Preferences preferences;


void MDNSRegister() {
  if (!enableWifi) return;
  Serial.println("[MDNS] Starting mDNS Service!");
  MDNS.begin(hostName.c_str());
  MDNS.addService("http", "tcp", 80);
}
