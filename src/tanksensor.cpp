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
#include <driver/dac.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <ESPAsync_WiFiManager.h>
#include <Preferences.h>
#include <driver/rtc_io.h>
#include <WiFi.h>

#include <NimBLEDevice.h>
#include <NimBLEBeacon.h>

#include "global.h"
#include "wifi-events.h"
#include "api-routes.h"

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : 
      Serial.println(F("[POWER] Wakeup caused by external signal using RTC_IO"));
      button1.pressed = true;
    break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println(F("[POWER] Wakeup caused by external signal using RTC_CNTL")); break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println(F("[POWER] Wakeup caused by timer"));
      uint64_t timeNow, timeDiff;
      timeNow = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
      timeDiff = timeNow - sleepTime;
      printf("Now: %" PRIu64 "ms, Duration: %" PRIu64 "ms\n", timeNow / 1000, timeDiff / 1000);
      delay(2000);
    break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println(F("[POWER] Wakeup caused by touchpad")); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println(F("[POWER] Wakeup caused by ULP program")); break;
    default : Serial.printf("[POWER] Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

uint64_t runtime() {
  return rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) / 1000;
}

void sleepOrDelay() {
  if (enableWifi || Tanklevel.isSetupRunning()) {
    // We don't want to, or can't go to deepsleep
    yield();
    delay(50);
  } else {
    delay(50); return;
    // We can save a lot of power by going into deepsleep
    // Thid disables WIFI and everything.
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    sleepTime = rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get());
    rtc_gpio_pullup_en(button1.PIN);
    rtc_gpio_pulldown_dis(button1.PIN);
    esp_sleep_enable_ext0_wakeup(button1.PIN, 0);

    preferences.end();
    Serial.println(F("[POWER] Sleeping..."));
    esp_deep_sleep_start();
  }
}

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}

uint8_t dacValue(uint8_t percentage) {
  uint8_t val = 0;
  if (percentage >= 0 && percentage <= 100) {
    float start = DAC_MIN_MVOLT / DAC_VCC * 255;   // startvolt / maxvolt * datapoints
    float end = DAC_MAX_MVOLT / DAC_VCC * 255;     // endvolt / maxvolt * datapoints
    val = round(start + (end-start) / 100.0 * percentage);
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, val);
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", val, (float)DAC_VCC/255*val);
  } else {
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 0);
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", 0, 0.00);
  }
  return val;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println(F("\n\n==== starting ESP32 setup() ===="));

  print_wakeup_reason();
  Serial.printf("[SETUP] Configure ESP32 to sleep for every %d Seconds\n", TIME_TO_SLEEP);

  Serial.printf("[GPIO] Configuration of GPIO %d as INPUT_PULLUP ... ", button1.PIN);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  Serial.println(F("done"));
  
  if (!LITTLEFS.begin(true)) {
    Serial.println(F("[FS] An Error has occurred while mounting LITTLEFS"));
    // reduce power consumption while having issues with NVS
    esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    // delay(1000); ESP.restart();
  }
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, String(NVS_NAMESPACE) + String("s1"));
  if (!preferences.begin(NVS_NAMESPACE)) {
    preferences.clear();
  }
  enableWifi = preferences.getBool("enableWifi", false);
  hostName = preferences.getString("hostName");
  if (hostName.isEmpty()) {
    hostName = "tanksensor";
    preferences.putString("hostName", hostName);
  }

  if (!Tanklevel.isConfigured()) {
    // we need to bring up WiFi to provide a convenient setup routine
    enableWifi = true;
  }
  
  if (!enableWifi && !startWifiConfigPortal) {
    Serial.println(F("[WIFI] Not starting WiFi!"));
  } else {
    WiFiRegisterEvents(WiFi);

    Serial.print(F("[WIFI] Starting Async_AutoConnect_ESP32_minimal on "));
    Serial.println(ARDUINO_BOARD);
    Serial.print(F("[WIFI] "));
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

    ESPAsync_WiFiManager wifiManager(&webServer, &dnsServer, hostName.c_str());
    if (!startWifiConfigPortal && wifiManager.WiFi_SSID() == "") {
      Serial.println(F("[WIFI] No AP credentials found, requesting Wifi configuration portal!"));
      startWifiConfigPortal = true;
    }
    if (startWifiConfigPortal) {
      String apName = "ESP_";
      apName += String((uint32_t)ESP.getEfuseMac(), HEX);
      apName.toUpperCase();
      Serial.printf("[WIFI] Starting configuration portal on AP SSID %s\n", apName.c_str());
      wifiManager.setConfigPortalTimeout(0);
      wifiManager.startConfigPortal(apName.c_str(), NULL);
      startWifiConfigPortal = false;
    } else {
      wifiManager.autoConnect();
    }
    APIRegisterRoutes();
    AsyncElegantOTA.begin(&webServer);
    webServer.begin();
    Serial.println(F("[WEB] HTTP server started"));

    WiFi.setAutoReconnect(true);
    MDNSRegister();
  } // end wifi

  createBleServer();
}

// Soft reset the ESP to start with setup() again, but without loosing RTC_DATA as it would be with ESP.reset()
void softReset() {
  if (enableWifi) {
    webServer.end();
    MDNS.end();
    WiFi.disconnect();
  }
  esp_sleep_enable_timer_wakeup(1);
  esp_deep_sleep_start();
}

void loop() {
  // if WiFi is down, try reconnecting
  if (enableWifi && runtime() - Timing.lastWifiCheck > Timing.wifiInterval) {
    Timing.lastWifiCheck = runtime();
    if (WiFi.getMode() == WIFI_MODE_AP) {
      Serial.println(F("[WIFI] Something went wrong here, we have an AP but no config portal!"));
      softReset();
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("[WIFI][%" PRIu64 "ms] Reconnecting to WiFi...\n", runtime());
      WiFi.reconnect();
    }
  }
  if (button1.pressed) {
    Serial.println(F("Button pressed!"));
    button1.pressed = false;
    if (!enableWifi) {
      // if no wifi is currently running, first button press will start it up
      preferences.putBool("enableWifi", true);
    } else {
      // if wifi is enabled, we start the config portal on next reboot
      startWifiConfigPortal = true;
    }
    preferences.end();
    softReset();
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
        Serial.println(F("[SENSOR] Unable to read data from sensor!"));
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
        updateBleCharacteristic(val);
        Serial.printf("[SENSOR] Current tank level %d percent, raw value is %d\n", val, Tanklevel.getMedian(true));
      } else {
        dacValue(0);
        val = Tanklevel.getMedian();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  sleepOrDelay();
}
