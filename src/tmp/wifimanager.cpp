/**
 *
 * Wifi Manager
 *
 * (c) 2022 Martin Verges
 *
**/
/*
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <wifimanager.h>
#include <ESPAsyncWebServer.h>
#include <algorithm>
#include <vector>
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

using std::vector;
using std::for_each;



bool WIFIMANAGER::begin() {
  int i = 0;
  for(const auto& cred: apList) {
    if (wifiMulti.addAP(cred.apName.c_str(), cred.apPass.c_str())) i++;
  }
  return i > 0;
}

void WIFIMANAGER::attachWebServer(AsyncWebServer &srv) {
  webServer = srv;
}

void WIFIMANAGER::detachWebserver() {
  //webServer = NULL;
}

bool WIFIMANAGER::addAp(String apName, String apPass) {
  apList.emplace_back(apName, apPass);

  return false;
}

bool WIFIMANAGER::connectTo(String apName, String apPass) {
  return false;
}

void WIFIMANAGER::loop() {
  if(wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
  }
}

bool WIFIMANAGER::registerApiEndpoints() {
  webServer.on((apiPrefix + "/list").c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
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
      wifiNet["rssi"] = rssi;
      wifiNet["channel"] = channel;
      yield();
    }

    serializeJson(jsonDoc, response);

    response->setCode(200);
    response->setContentLength(measureJson(jsonDoc));
    request->send(response);
  });

  webServer.on((apiPrefix + "/status").c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
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

    serializeJson(jsonDoc, response);

    response->setCode(200);
    response->setContentLength(measureJson(jsonDoc));
    request->send(response);
  });
  return true;
}

*/