/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LITTLEFS.h>
#include "ble.h"

extern bool enableWifi;
extern bool enableBle;
extern bool enableMqtt;
extern bool enableDac;

void APIRegisterRoutes() {
  webServer.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    request->send(200, "application/json", "{\"message\":\"Resetting the sensor!\"}");
    request->send(response);
    yield();
    delay(250);
    ESP.restart();
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
      DynamicJsonDocument jsonBuffer(1024);
      deserializeJson(jsonBuffer, (const char*)data);

      String hostname = jsonBuffer["hostname"].as<String>();
      if (!hostname || hostname.length() < 3 || hostname.length() > 32) {
        // TODO: Add better checks according to RFC hostnames
        request->send(422, "application/json", "{\"message\":\"Invalid hostname!\"}");
        return;
      } else {
        preferences.putString("hostName", hostname);
      }

      if (preferences.putBool("enableWifi", jsonBuffer["enablewifi"].as<boolean>())) {
        enableWifi = jsonBuffer["enablewifi"].as<boolean>();
      }
      if (preferences.putBool("enableBle", jsonBuffer["enableble"].as<boolean>())) {
        if (enableBle) stopBleServer();
        enableBle = jsonBuffer["enableble"].as<boolean>();
        if (enableBle) createBleServer(hostName);
        yield();
      }
      if (preferences.putBool("enableWifi", jsonBuffer["enablewifi"].as<boolean>())) {
        enableWifi = jsonBuffer["enablewifi"].as<boolean>();
      }
      if (preferences.putBool("enableDac", jsonBuffer["enabledac"].as<boolean>())) {
        enableDac = jsonBuffer["enabledac"].as<boolean>();
      }

      // MQTT Settings
      preferences.putUInt("mqttPort", jsonBuffer["mqttport"].as<uint16_t>());
      preferences.putString("mqttHost", jsonBuffer["mqtthost"].as<String>());
      preferences.putString("mqttTopic", jsonBuffer["mqtttopic"].as<String>());
      preferences.putString("mqttUser", jsonBuffer["mqttuser"].as<String>());
      preferences.putString("mqttPass", jsonBuffer["mqttpass"].as<String>());
      if (preferences.putBool("enableMqtt", jsonBuffer["enablemqtt"].as<boolean>())) {
        if (enableMqtt) Mqtt.disconnect(true);
        enableMqtt = jsonBuffer["enablemqtt"].as<boolean>();
        if (enableMqtt) {
          Mqtt.prepare();
          Mqtt.connect();
        }
      }
      
      request->send(200, "application/json", "{\"message\":\"New hostname stored in NVS, reboot required!\"}");

    } else if (request->url() == "/api/level/data" && request->method() == HTTP_POST) {
      DynamicJsonDocument jsonBuffer(2048);
      deserializeJson(jsonBuffer, (const char*)data);

      Tanklevel.writeSingleEntrytoNVS(255, jsonBuffer["setupDone"].as<boolean>());

      JsonArray array = jsonBuffer["data"].as<JsonArray>();
      uint8_t i = 0;
      for (JsonVariant v : array) {
          Tanklevel.writeSingleEntrytoNVS(i, v.as<int>());
          yield();
          i++;
          if (i > 100) break;
      }
      request->send(200, "application/json", "{\"message\":\"Wrote level config to NVS, restarting now!\"}");
      
      yield();
      delay(250);
      ESP.restart();
    }
  });

  webServer.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<1024> doc;

      doc["hostname"] = hostName;
      doc["enablewifi"] = enableWifi;
      doc["enableble"] = enableBle;
      doc["enabledac"] = enableDac;

      // MQTT
      doc["enablemqtt"] = enableMqtt;
      doc["mqttport"] = preferences.getUInt("mqttPort", 1883);
      doc["mqtthost"] = preferences.getString("mqttHost", "");
      doc["mqtttopic"] = preferences.getString("mqttTopic", "");
      doc["mqttuser"] = preferences.getString("mqttUser", "");
      doc["mqttpass"] = preferences.getString("mqttPass", "");

      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(415, "text/plain", "Unsupported Media Type");
  });

  webServer.on("/api/rawvalue", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<16> doc;
      doc["raw"] = Tanklevel.getSensorMedianValue(true);
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(200, "text/plain", (String)Tanklevel.getSensorMedianValue(true));
  });

  webServer.on("/api/level/current", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->contentType() == "application/json") {
      String output;
      StaticJsonDocument<16> doc;
      doc["levelPercent"] = Tanklevel.getCalculatedPercentage(true);
      serializeJson(doc, output);
      request->send(200, "application/json", output);
    } else request->send(200, "text/plain", (String)Tanklevel.getCalculatedPercentage(true));
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

  webServer.on("/api/level/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(3072);
    json["setupDone"] = Tanklevel.isConfigured();

    const size_t CAPACITY = JSON_ARRAY_SIZE(101);
    StaticJsonDocument<CAPACITY> doc;
    JsonArray array = doc.to<JsonArray>();
    for (int i = 0; i <= 100; i++) array.add(Tanklevel.getLevelData(i));
    json["data"] = array;

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
