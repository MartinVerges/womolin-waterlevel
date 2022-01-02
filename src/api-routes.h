/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2021 Martin Verges
 *
**/

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LITTLEFS.h>

void APIRegisterRoutes() {
  webServer.on("/api/wifi-list", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDoc(2048);
    JsonArray wifiList = jsonDoc.createNestedArray("wifiList");

    int scanResult;
    String ssid;
    uint8_t encryptionType;
    int32_t rssi;
    uint8_t* bssid;
    int32_t channel;

    scanResult = WiFi.scanNetworks(false, true);
    for (int8_t i = 0; i < scanResult; i++) {
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel);

      JsonObject wifiNet = wifiList.createNestedObject();
      wifiNet["ssid"] = ssid;
      wifiNet["encryptionType"] = encryptionType;
      wifiNet["rssi"] = String(rssi);
      wifiNet["channel"] = String(channel);
      yield();
    }

    serializeJson(jsonDoc, *response);

    response->setCode(200);
    response->setContentLength(measureJson(jsonDoc));
    request->send(response);
  });

  webServer.on("/api/wifi-status", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDoc(1024);

    jsonDoc["ip"] = WiFi.localIP().toString();
    jsonDoc["gw"] = WiFi.gatewayIP().toString();
    jsonDoc["nm"] = WiFi.subnetMask().toString();

    jsonDoc["hostname"] = WiFi.getHostname();
    jsonDoc["signalStrengh"] = WiFi.RSSI();
    
    jsonDoc["chipModel"] = ESP.getChipModel();
    jsonDoc["chipRevision"] = ESP.getChipRevision();
    jsonDoc["chipCores"] = ESP.getChipCores();
    
    jsonDoc["getHeapSize"] = ESP.getHeapSize();
    jsonDoc["freeHeap"] = ESP.getFreeHeap();

    serializeJson(jsonDoc, *response);

    response->setCode(200);
    response->setContentLength(measureJson(jsonDoc));
    request->send(response);
  });

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
    if (request->url() == "/api/setup/values" && request->method() == HTTP_POST) {
      // Do a simple linear tank level setup using lower+upper reading
      DynamicJsonDocument jsonBuffer(64);
      deserializeJson(jsonBuffer, (const char*)data);

      if (!jsonBuffer["lower"].is<int>() || !jsonBuffer["upper"].is<int>()) {
        request->send(422, "text/plain", "Invalid data");
        return;
      }
      if (!Tanklevel.setupFrom2Values(jsonBuffer["lower"], jsonBuffer["upper"])) {
        request->send(500, "application/json", "{\"message\":\"Unable to process data\"}");
      } else request->send(200, "application/json", "{\"message\":\"Setup completed\"}");

    } else if (request->url() == "/api/config" && request->method() == HTTP_POST) {
      // Configure the hostname of the sensor
      DynamicJsonDocument jsonBuffer(256);
      deserializeJson(jsonBuffer, (const char*)data);

      String hostname = jsonBuffer["hostname"].as<String>();
      bool enablewifi = jsonBuffer["enablewifi"].as<boolean>();

      if (!hostname || hostname.length() < 3 || hostname.length() > 32) {
        // TODO: Add better checks according to RFC hostnames
        request->send(422, "text/plain", "Invalid hostname");
        return;
      }

      int s = 0;
      if (preferences.putString("hostName", hostname)) s++;
      if (preferences.putBool("enableWifi", enablewifi)) {
        enableWifi = enablewifi;
        s++;
      }
      if (s == 2) {
        request->send(200, "application/json", "{\"message\":\"New hostname stored in NVS, reboot required!\"}");
      } else request->send(200, "text/plain", "New hostname stored in NVS");
    }
  });
  webServer.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<256> doc;
      doc["hostname"] = hostName;
      doc["enablewifi"] = enableWifi;
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(415, "text/plain", "Unsupported Media Type");
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
  webServer.on("/api/wifi/info", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument json(1024);
      json["status"] = "ok";
      json["ssid"] = WiFi.SSID();
      json["ip"] = WiFi.localIP().toString();
      serializeJson(json, *response);
      request->send(response);
  });
  webServer.on("/api/wifi/disable", HTTP_POST, [](AsyncWebServerRequest *request) {
    enableWifi = false;
    preferences.putBool("enableWifi", false);
    if (request->contentType() == "application/json") { 
      request->send(200, "application/json", "{\"message\":\"Shutting down WiFi\"}");
    } else request->send(200, "text/plain", "Shutting down WiFi");
  });
  webServer.on("/api/wifi/enable", HTTP_POST, [](AsyncWebServerRequest *request) {
    enableWifi = true;
    preferences.putBool("enableWifi", true);
    if (request->contentType() == "application/json") { 
      request->send(200, "application/json", "{\"message\":\"Permanently enabling WiFi\"}");
    } else request->send(200, "text/plain", "Permanently enabling WiFi");
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

  webServer.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      if (request->contentType() == "application/json") {
        request->send(404, "application/json", "{\"message\":\"Not found\"}");
      } else request->send(404, "text/plain", "Not found");
    }
  });
}
