#if !(defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

// Fix an issue with the HX711 library on ESP32
#undef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32 true
#undef USE_LITTLEFS
#define USE_LITTLEFS true

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsync_WiFiManager.h> 
#include <LITTLEFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <sensor.h>

#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif

#define NVS_NAMESPACE "tanksensor"

const int GPIO_HX711_DOUT = 26;            // GPIO pin to use for DT or DOUT
const int GPIO_HX711_SCK = 25;             // GPIO pin to use for SCK
const int webserverPort = 80;              // Start the Webserver on this port

unsigned long millisStarted = 0;           // timer to run parts only at given interval

TANKLEVEL Tanklevel;

struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button1 = {21, false};              // Run the setup 
Button button2 = {22, false};              // unused

String wifi_ssid = "ESP_" + String((uint32_t)ESP.getEfuseMac(), HEX);
String wifi_pass = "";
String hostName = "my-tanksensor";
DNSServer dnsServer;

AsyncWebServer webServer(webserverPort);
AsyncEventSource events("/events");


void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}
void IRAM_ATTR ISR_button2() {
  button2.pressed = true;
}

void setup() {
  Serial.begin(115200);
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE);
  
  if(!LITTLEFS.begin(true)){
    Serial.println("An Error has occurred while mounting LITTLEFS");
    ESP.restart();
  }

  Serial.print("\nStarting Async_AutoConnect_ESP32_minimal on " + String(ARDUINO_BOARD));
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "Async_AutoConnect");
  ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192,168,0,1), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  ESPAsync_wifiManager.autoConnect("AutoConnectAP");
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.print(F("Connected. Local IP: ")); 
    Serial.println(WiFi.localIP()); 
  } else {
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status())); 
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
  
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
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
  Serial.println("HTTP server started");

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, ISR_button2, FALLING);
}

void loop() {
  if (button1.pressed) {
    if (Tanklevel.isSetupRunning()) {
      Serial.println("User requested end of setup...");
      Tanklevel.endLevelSetup();
    } else {
      Serial.println("User requested start of setup...");
      Tanklevel.beginLevelSetup();
    }
    button1.pressed = false;
  }

  if (button2.pressed) {
    button2.pressed = false;
    //ESPAsync_wifiManager.startConfigPortal();
    //Tanklevel.abortLevelSetup();
    Tanklevel.printData();
  }
  
  if (Tanklevel.isSetupRunning()) {
    // run the level setup
    if (millis() - millisStarted >= 100) {
      millisStarted = millis();
      int val = Tanklevel.runLevelSetup();
      if (val > 0) {
        events.send(String(val).c_str(), "setup", millis());
      } else {
        events.send("Unable to read data from sensor!", "setuperror", millis());
      }
    }
  } else {
    // run regular operation
    if (millis() - millisStarted > 1000) {
      millisStarted = millis();
      int val = -1;
      if (Tanklevel.isConfigured()) {
        val = Tanklevel.getPercentage();
        events.send(String(val).c_str(), "level", millis());
        Serial.printf("Tank fill status = %d\n", val);
      } else {
        val = Tanklevel.getMedian();
        Serial.printf("Tanklevel Sensor not configured, please run the setup! Raw value = %d\n", val);
      }
    }
  }
  delay(25);
}
