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
#define TIME_TO_SLEEP    60                // WakeUp at least once a minute
#define ESP_DRD_USE_LITTLEFS    true       // Write double restart information to FS

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <esp_wifi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsync_WiFiManager.h> 
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <sensor.h>
#include <ESP_DoubleResetDetector.h>

RTC_DATA_ATTR int bootCount = 0;           // Counter to keep track

const int GPIO_DAC_LEVEL_OUT = 25;         // Provide current level using a 1-3V level
const int GPIO_HX711_DOUT = 33;            // GPIO pin to use for DT or DOUT
const int GPIO_HX711_SCK = 32;             // GPIO pin to use for SCK
const int webserverPort = 80;              // Start the Webserver on this port
const String NVS_NAMESPACE = "tanksensor"; // Preferences.h namespace to store settings

struct timing_t {
  // Wifi interval in loop()
  unsigned long lastWifiCheck = 0;         // last millis() from WifiCheck
  unsigned int wifiInterval = 1000;        // Interval in ms to execute code

  // Sensor data in loop()
  unsigned long lastSensorRead = 0;        // last millis() from Sensor read
  unsigned int sensorInterval = 1000;      // Interval in ms to execute code

  // Setup executing in loop()
  unsigned long lastSetupRead = 0;         // last millis() from Setup run
  unsigned int setupInterval = 250;        // Interval in ms to execute code
} Timing;

bool startWifiConfigPortal = false;        // Start the config portal on setup()

TANKLEVEL Tanklevel;

struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button1 = {4, false};               // Run the setup (use a RTC GPIO)

String hostName = "tanksensor-" + String((uint32_t)ESP.getEfuseMac(), HEX);
DNSServer dnsServer;
DoubleResetDetector* drd;
AsyncWebServer webServer(webserverPort);
AsyncEventSource events("/events");

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

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] Connected successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("[WIFI] WiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] Disconnected from WiFi access point with Reason:");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.reconnect();
}

void WiFIApStarted(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] AP mode started!");
}

void WiFIApStopped(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] AP mode stopped!");
}

void add_mdns_services() {
  Serial.println("[MDNS] Starting mDNS Service!");
  MDNS.begin(hostName.c_str());
  MDNS.addService("http", "tcp", 80);
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
  Serial.println("done");

  Serial.printf("[GPIO] Configuration of GPIO %d as OUTPUT ... ", GPIO_DAC_LEVEL_OUT);
  pinMode(GPIO_DAC_LEVEL_OUT, ANALOG);
  dacWrite(GPIO_DAC_LEVEL_OUT, 0); //255= 3.3V 128=1.65V
  Serial.println("done");
  

  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE);
  
  if (!LITTLEFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LITTLEFS");
    ESP.restart();
  }

  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(WiFIApStarted, SYSTEM_EVENT_AP_START);
  WiFi.onEvent(WiFIApStopped, SYSTEM_EVENT_AP_STOP);

  Serial.printf("Starting Async_AutoConnect_ESP32_minimal on %s\n", ARDUINO_BOARD);
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

  webServer.on("/api/setup/start", HTTP_POST, [](AsyncWebServerRequest *request) {
    Tanklevel.setStartAsync();
    if (request->contentType() == "application/json") {
      request->send(200, "application/json", "{\"message\":\"Begin of Setup requested\"}");
    } else request->send(200, "text/plain", "Begin of Setup requested");
  });
  webServer.on("/api/setup/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<16> doc;
      doc["setupIsRunning"] = Tanklevel.isSetupRunning();
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(200, "text/plain", String(Tanklevel.isSetupRunning()));
  });
  webServer.on("/api/setup/end", HTTP_POST, [](AsyncWebServerRequest *request) {
    Tanklevel.setEndAsync();
    if (request->contentType() == "application/json") {
      request->send(200, "application/json", "{\"message\":\"End of Setup requested\"}");
    } else request->send(200, "text/plain", "End of Setup requested");
  });
  webServer.on("/api/setup/abort", HTTP_POST, [](AsyncWebServerRequest *request) {
    Tanklevel.setAbortAsync();
    if (request->contentType() == "application/json") {
      request->send(200, "application/json", "{\"message\":\"Abort requested\"}");
    } else request->send(200, "text/plain", "Abort requested");
  });
  webServer.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    Serial.println("Running");
    if (request->url() == "/api/setup/values" && request->method() == HTTP_POST) {
      DynamicJsonDocument jsonBuffer(64);
      deserializeJson(jsonBuffer, (const char*)data);

      if (!jsonBuffer["lower"].is<int>() || !jsonBuffer["upper"].is<int>()) {
        request->send(422, "text/plain", "Invalid data");
        return;
      }
      if (Tanklevel.setupFrom2Values(jsonBuffer["lower"], jsonBuffer["upper"])) {
        request->send(500, "text/plain", "Unable to process data");
      } else request->send(200, "application/json", "{\"message\":\"Setup completed\"}");
    }
  });
  webServer.on("/api/rawvalue", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<16> doc;
      doc["raw"] = Tanklevel.getMedian(true);
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(200, "text/plain", (String)Tanklevel.getMedian(true));
  });
  webServer.on("/api/level", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<16> doc;
      doc["levelPercent"] = Tanklevel.getPercentage(true);
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(200, "text/plain", (String)Tanklevel.getPercentage(true));
  });
  webServer.on("/api/esp/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
  webServer.on("/api/esp/cores", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getChipCores()));
  });
  webServer.on("/api/esp/freq", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getCpuFreqMHz()));
  });
  webServer.on("/api/wifi-info", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument json(1024);
      json["status"] = "ok";
      json["ssid"] = WiFi.SSID();
      json["ip"] = WiFi.localIP().toString();
      serializeJson(json, *response);
      request->send(response);
  });
  webServer.on("/api/level-info", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(3072);
    for (int i = 0; i < 100; i++) json["val" + String(i)] = Tanklevel.getLevelData(i);
    serializeJson(json, *response);
    request->send(response);
  });
  
  webServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");
  
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("[WEB] Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
  });
  webServer.addHandler(&events);

  AsyncElegantOTA.begin(&webServer);    // Start ElegantOTA
  
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      request->send(404, "application/json", "{\"message\":\"Not found\"}");
    } else request->send(404, "text/plain", "Not found");
  });
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
        dacWrite(GPIO_DAC_LEVEL_OUT, 255/100*val); //255= 3.3V 128=1.65V
        Serial.printf("[SENSOR] Current tank level %d percent, raw value is %d\n", val, Tanklevel.getMedian(true));
      } else {
        val = Tanklevel.getMedian();
        Serial.printf("[SENSOR] Tanklevel not configured, please run the setup! Raw sensor value = %d\n", val);
      }
    }
  }
  delay(25);
}
