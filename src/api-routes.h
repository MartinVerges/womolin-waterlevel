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
#include <LittleFS.h>

void APIRegisterRoutes() {
  webServer.on("/api/wifi/disable", HTTP_POST, [](AsyncWebServerRequest *request) {
    enableWifi = false;
    if (request->contentType() == "application/json") { 
      request->send(200, "application/json", "{\"message\":\"Shutting down WiFi\"}");
    } else request->send(200, "text/plain", "Shutting down WiFi");
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

  webServer.onNotFound([](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      request->send(404, "application/json", "{\"message\":\"Not found\"}");
    } else request->send(404, "text/plain", "Not found");
  });
}
