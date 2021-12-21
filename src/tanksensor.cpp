#if !(defined(ESP32))
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

// Fix an issue with the HX711 library on ESP32
#if !(defined(ARDUINO_ARCH_ESP32))
  #define ARDUINO_ARCH_ESP32 true
#endif
#undef USE_LITTLEFS
#define USE_LITTLEFS true

#define uS_TO_S_FACTOR   1000000           // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP    1                 // WakeUp interval
#define ESP_DRD_USE_LITTLEFS    true       // Write double restart information to FS

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <ESPAsync_WiFiManager.h> 
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <wifi-events.h>

#include "global.h"
#include "api-routes.h"

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("[POWER] Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("[POWER] Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("[POWER] Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("[POWER] Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("[POWER] Wakeup caused by ULP program"); break;
    default : Serial.printf("[POWER] Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}

void add_mdns_services() {
  Serial.println("[MDNS] Starting mDNS Service!");
  MDNS.begin(hostName.c_str());
  MDNS.addService("http", "tcp", 80);
}

void dacValue(uint8_t percentage) {
  if (percentage >= 0 && percentage <= 100) {
    float start = DAC_MIN_MVOLT / DAC_VCC * 255;   // startvolt / maxvolt * datapoints
    float end = DAC_MAX_MVOLT / DAC_VCC * 255;     // endvolt / maxvolt * datapoints
    uint8_t val = round(start + (end-start) / 100 * percentage);
    dacWrite(GPIO_DAC_LEVEL_OUT, val);  // ESP32 values 255= 3.3V 128= 1.65V
  } else {
    dacWrite(GPIO_DAC_LEVEL_OUT, 0);
  }
}

void setup() {
  ++bootCount;  // increment on each boot
  Serial.begin(115200);
  Serial.println("\n\n==== starting ESP32 setup() ====");
  Serial.printf("[SETUP] Boot number: %d\n", bootCount);

  print_wakeup_reason();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.printf("[SETUP] Configure ESP32 to sleep for every %d Seconds\n", TIME_TO_SLEEP);

  Serial.printf("[GPIO] Configuration of GPIO %d as INPUT_PULLUP ... ", button1.PIN);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  esp_sleep_enable_ext0_wakeup(button1.PIN, 0);
  Serial.println("done");

  Serial.printf("[GPIO] Configuration of GPIO %d as OUTPUT ... ", GPIO_DAC_LEVEL_OUT);
  pinMode(GPIO_DAC_LEVEL_OUT, ANALOG);
  Serial.println("done");
  
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE);
  
  if (!LITTLEFS.begin(true)) {
    Serial.println("[FS] An Error has occurred while mounting LITTLEFS");
    ESP.restart();
  }

  WiFiRegisterEvents(WiFi);

  Serial.printf("[WIFI] Starting Async_AutoConnect_ESP32_minimal on %s\n", ARDUINO_BOARD);
  Serial.printf("[WIFI] %s\n", ESP_ASYNC_WIFIMANAGER_VERSION);

  drd = new DoubleResetDetector(10, 0);
  if (drd->detectDoubleReset()) { 
    Serial.println("[DRD] Detected double reset or initial run, requesting Wifi configuration portal!");
    startWifiConfigPortal = true;
  }

  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, hostName.c_str());
  if (!startWifiConfigPortal && ESPAsync_wifiManager.WiFi_SSID() == "") {
    Serial.println("[WIFI] No AP credentials found, requesting Wifi configuration portal!");
    startWifiConfigPortal = true;
  }
  if (startWifiConfigPortal) {
    drd->stop();
    Serial.println("[WIFI] Starting configuration portal");
    ESPAsync_wifiManager.startConfigPortal();
  } else {
    ESPAsync_wifiManager.autoConnect();
  }

  APIRegisterRoutes();
  AsyncElegantOTA.begin(&webServer);    // Start ElegantOTA
  webServer.begin();
  Serial.println("[WEB] HTTP server started");

  add_mdns_services();
}

void loop() {
  drd->loop(); // refresh timeout for double reset detection

  // if WiFi is down, try reconnecting
  if (millis() - Timing.lastWifiCheck > Timing.wifiInterval) {
    Timing.lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  if (button1.pressed) {
    button1.pressed = false;
    WiFi.disconnect();
    webServer.end();
    ESP.restart();
  }
  
  if (Tanklevel.isSetupRunning()) {
    // run the level setup
    if (millis() - Timing.lastSetupRead >= Timing.setupInterval) {
      Timing.lastSetupRead = millis();
      int val = Tanklevel.runLevelSetup();
      if (val > 0) {
        events.send(String(val).c_str(), "setup", millis());
      } else {
        events.send("Unable to read data from sensor!", "setuperror", millis());
        Serial.println("[SENSOR] Unable to read data from sensor!");
      }
    }
  } else {
    // run regular operation
    if (millis() - Timing.lastSensorRead > Timing.sensorInterval) {
      Timing.lastSensorRead = millis();
      int val = -1;
      if (Tanklevel.isConfigured()) {
        val = Tanklevel.getPercentage();
        events.send(String(val).c_str(), "level", millis());
        dacValue(val); 
        Serial.printf("[SENSOR] Current tank level %d percent, raw value is %d\n", val, Tanklevel.getMedian(true));
      } else {
        val = Tanklevel.getMedian();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  // delay(25);
  Serial.println("sleep");
  esp_light_sleep_start();
}
