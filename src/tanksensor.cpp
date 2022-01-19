/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#if !(defined(ESP32))
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#define uS_TO_S_FACTOR   1000000           // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP    10                 // WakeUp interval

// Fix an issue with the HX711 library on ESP32
#if !(defined(ARDUINO_ARCH_ESP32))
  #define ARDUINO_ARCH_ESP32 true
#endif
#undef USE_LITTLEFS
#define USE_LITTLEFS true

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <esp_wifi.h>
#include <ESPAsync_WiFiManager.h>
#include <WiFi.h>
#include <Preferences.h>

// Power Management
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}

#include "global.h"
#include "wifi-events.h"
#include "api-routes.h"
#include "mqtt.h"
#include "ble.h"
#include "dac.h"

extern AsyncMqttClient mqttClient;
extern String mqttTopic;

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}

void deepsleepForSeconds(int seconds) {
    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

// Check if a feature is enabled, that prevents the
// deep sleep mode of our ESP32 chip.
void sleepOrDelay() {
  if (enableWifi || enableBle || enableMqtt || Tanklevel.isSetupRunning()) {
    yield();
    delay(50);
  } else {
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
    // Reduce power consumption while having issues with NVS
    // This won't fix the problem, a check of the sensor log is required
    deepsleepForSeconds(5);
  }
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, String(NVS_NAMESPACE) + String("s1"));
  if (!preferences.begin(NVS_NAMESPACE)) {
    preferences.clear();
  }

  // Load Settings from NVS
  hostName = preferences.getString("hostName");
  if (hostName.isEmpty()) {
    hostName = "tanksensor";
    preferences.putString("hostName", hostName);
  }
  enableWifi = preferences.getBool("enableWifi", true);
  enableBle = preferences.getBool("enableBle", true);
  enableDac = preferences.getBool("enableDac", true);

  // Only enable Mqtt if Wifi is enabled as well
  if (enableWifi) {
    enableMqtt = preferences.getBool("enableMqtt", false);
    if (enableMqtt) {
      prepareMqtt(preferences);
    } else Serial.println(F("[MQTT] Publish to MQTT is disabled."));
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
    MDNSBegin(hostName);
  } // end wifi

  if (enableBle) createBleServer(hostName);
  else Serial.println(F("[BLE] Bluetooth low energy is disabled."));
}

// Soft reset the ESP to start with setup() again, but without loosing RTC_DATA as it would be with ESP.reset()
void softReset() {
  if (enableWifi) {
    webServer.end();
    MDNSEnd();
    mqttClient.disconnect();
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
    Serial.println(F("[EVENT] Button pressed!"));
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
        val = Tanklevel.getCalculatedPercentage();
        events.send(String(val).c_str(), "level", runtime());
        if (enableDac) dacValue(val);
        if (enableBle) updateBleCharacteristic(val);
        if (enableMqtt && mqttTopic.length() > 0) {
          mqttClient.publish((mqttTopic + "/tanklevel").c_str(), 0, true, String(val).c_str());
        }
        Serial.printf("[SENSOR] Current tank level %d percent, raw value is %d\n", val, Tanklevel.getSensorMedianValue(true));
      } else {
        if (enableDac) dacValue(0);
        if (enableBle) updateBleCharacteristic(0);
        val = Tanklevel.getSensorMedianValue();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  sleepOrDelay();
}
