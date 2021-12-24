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

    preferences.end();
    Serial.println("[POWER] Sleeping...");
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
  Serial.println("\n\n==== starting ESP32 setup() ====");

  print_wakeup_reason();
  Serial.printf("[SETUP] Configure ESP32 to sleep for every %d Seconds\n", TIME_TO_SLEEP);

  Serial.printf("[GPIO] Configuration of GPIO %d as INPUT_PULLUP ... ", button1.PIN);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  Serial.println("done");
  
  if (!LITTLEFS.begin(true)) {
    Serial.println("[FS] An Error has occurred while mounting LITTLEFS");
    // reduce power consumption while having issues with NVS
    esp_sleep_enable_timer_wakeup(1 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    // delay(1000); ESP.restart();
  }
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE+"s1");
  preferences.begin(NVS_NAMESPACE.c_str());
  enableWifi = preferences.getBool("enableWifi", false);
  hostName = preferences.getString("hostName", "tanksensor");

  if (!Tanklevel.isConfigured()) {
    // we need to bring up WiFi to provide a convenient setup routine
    enableWifi = true;
  }
  
  if (!enableWifi && !startWifiConfigPortal) {
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
      String apName = "ESP_";
      apName += String((uint32_t)ESP.getEfuseMac(), HEX);
      ESPAsync_wifiManager.startConfigPortal(apName.c_str(), NULL);
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
    if (!enableWifi) {
      // if no wifi is currently running, first button press will start it up
      preferences.putBool("enableWifi", true);
    } else {
      // if wifi is enabled, we start the config portal on next reboot
      WiFi.disconnect();
      webServer.end();
      startWifiConfigPortal = true;
    }
    preferences.end();
    // ESP.restart(); // causes RTC_DATA_ATTR data reset!
    esp_sleep_enable_timer_wakeup(1);
    esp_deep_sleep_start();
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
        dacValue(0);
        val = Tanklevel.getMedian();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  sleepOrDelay();
}
