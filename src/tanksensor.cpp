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
#define TIME_TO_SLEEP    10                 // WakeUp interval

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <ESPAsync_WiFiManager.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include "global.h"
#include "wifi-events.h"
#include "api-routes.h"

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : 
      Serial.println("[POWER] Wakeup caused by external signal using RTC_IO");
      button1.pressed = true;
    break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("[POWER] Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("[POWER] Wakeup caused by timer");
      uint64_t timeNow, timeDiff;
      timeNow = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
      timeDiff = timeNow - sleepTime;
      printf("Now: %" PRIu64 "ms, Duration: %" PRIu64 "ms\n", timeNow / 1000, timeDiff / 1000);
      delay(2000);
    break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("[POWER] Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("[POWER] Wakeup caused by ULP program"); break;
    default : Serial.printf("[POWER] Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

uint64_t runtime() {
  return rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
}

void sleepOrDelay() {
  if (enableWifi || Tanklevel.isSetupRunning()) {
    // We don't want to, or can't go to deepsleep
    delay(25);
  } else {
    // We can save a lot of power by going into deepsleep
    // Thid disables WIFI and everything.
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
    rtc_gpio_pullup_en(button1.PIN);
    rtc_gpio_pulldown_dis(button1.PIN);
    esp_sleep_enable_ext0_wakeup(button1.PIN, 0);
    Serial.println("[POWER] Sleeping...");
    esp_deep_sleep_start();
  }
}

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}

void dacValue(uint8_t percentage) {
  if (percentage >= 0 && percentage <= 100) {
    float start = DAC_MIN_MVOLT / DAC_VCC * 255;   // startvolt / maxvolt * datapoints
    float end = DAC_MAX_MVOLT / DAC_VCC * 255;     // endvolt / maxvolt * datapoints
    uint8_t val = round(start + (end-start) / 100 * percentage);
    dacWrite(GPIO_DAC_LEVEL_OUT, val);            // ESP32 values 255= 3.3V 128= 1.65V
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", val, (float)DAC_VCC/255*val);
  } else {
    dacWrite(GPIO_DAC_LEVEL_OUT, 0);
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", 0, 0.00);
  }
}

void setup() {
  delay(250); // avoid some hanging or stuck bugs xD

//  rtc_gpio_pullup_dis(button1.PIN);
//  rtc_gpio_hold_dis(button1.PIN);
  Serial.begin(115200);
  Serial.println("\n\n==== starting ESP32 setup() ====");

  print_wakeup_reason();
  Serial.printf("[SETUP] Configure ESP32 to sleep for every %d Seconds\n", TIME_TO_SLEEP);

  Serial.printf("[GPIO] Configuration of GPIO %d as INPUT_PULLUP ... ", button1.PIN);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  Serial.println("done");
  
  Serial.printf("[GPIO] Configuration of GPIO %d as OUTPUT ... ", GPIO_DAC_LEVEL_OUT);
  pinMode(GPIO_DAC_LEVEL_OUT, ANALOG);
  Serial.println("done");
  
  if (!LITTLEFS.begin(true)) {
    Serial.println("[FS] An Error has occurred while mounting LITTLEFS");
    ESP.restart();
  }
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE);
  if (!Tanklevel.isConfigured()) {
    // we need to bring up WiFi to provide a convenient setup routine
    enableWifi = true;
  }
  
  if (!enableWifi) {
    Serial.println("[WIFI] Not starting WiFi!");
  } else {
    WiFiRegisterEvents(WiFi);

    Serial.printf("[WIFI] Starting Async_AutoConnect_ESP32_minimal on %s\n", ARDUINO_BOARD);
    Serial.printf("[WIFI] %s\n", ESP_ASYNC_WIFIMANAGER_VERSION);

    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, hostName.c_str());
    if (!startWifiConfigPortal && ESPAsync_wifiManager.WiFi_SSID() == "") {
      Serial.println("[WIFI] No AP credentials found, requesting Wifi configuration portal!");
      startWifiConfigPortal = true;
    }
    if (startWifiConfigPortal) {
      Serial.println("[WIFI] Starting configuration portal");
      ESPAsync_wifiManager.startConfigPortal();
      startWifiConfigPortal = false;
    } else {
      ESPAsync_wifiManager.autoConnect();
    }
    APIRegisterRoutes();
    AsyncElegantOTA.begin(&webServer);    // Start ElegantOTA
    webServer.begin();
    Serial.println("[WEB] HTTP server started");

    MDNSRegister();
  } // end wifi
}

void loop() {
  // if WiFi is down, try reconnecting
  if (enableWifi && runtime() - Timing.lastWifiCheck > Timing.wifiInterval) {
    Timing.lastWifiCheck = runtime();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WIFI] Reconnecting to WiFi...");
      WiFi.reconnect();
    }
  }

  if (button1.pressed) {
    Serial.println("Button pressed!");
    button1.pressed = false;
    startWifiConfigPortal = true;
    if (enableWifi) {
      WiFi.disconnect();
      webServer.end();
    }
    enableWifi = true;
    ESP.restart();
  }
  
  if (Tanklevel.isSetupRunning()) {
    // run the level setup
    if (runtime() - Timing.lastSetupRead >= Timing.setupInterval) {
      Timing.lastSetupRead = runtime();
      int val = Tanklevel.runLevelSetup();
      if (val > 0) {
        events.send(String(val).c_str(), "setup", runtime());
      } else {
        events.send("Unable to read data from sensor!", "setuperror", runtime());
        Serial.println("[SENSOR] Unable to read data from sensor!");
      }
    }
  } else {
    // run regular operation
    if (runtime() - Timing.lastSensorRead > Timing.sensorInterval) {
      Timing.lastSensorRead = runtime();
      int val = -1;
      if (Tanklevel.isConfigured()) {
        val = Tanklevel.getPercentage();
        events.send(String(val).c_str(), "level", runtime());
        dacValue(val); 
        Serial.printf("[SENSOR] Current tank level %d percent, raw value is %d\n", val, Tanklevel.getMedian(true));
      } else {
        val = Tanklevel.getMedian();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  sleepOrDelay();
}
