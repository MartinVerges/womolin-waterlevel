/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <WiFi.h>
#include "MQTTclient.h"
#include <ESPmDNS.h>

extern bool enableWifi;
extern bool enableMqtt;

void MDNSBegin(String hostname) {
  if (!enableWifi) return;
  Serial.println("[MDNS] Starting mDNS Service!");
  MDNS.begin(hostname.c_str());
  MDNS.addService("http", "tcp", 80);
}

void MDNSEnd() {
  MDNS.end();
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("[WIFI] WiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
//  if (enableMqtt) connectToMqtt();
//  else Serial.println("[MQTT] disabled, not starting!");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] Connected successfully!");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  //if (enableMqtt) MQTTclient();  
  Serial.println("[WIFI] Disconnected from WiFi access point with Reason:");
  Serial.println(info.disconnected.reason);
}

void WiFiApStarted(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] AP mode started!");
}

void WiFiApStopped(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] AP mode stopped!");
}

void WiFiRegisterEvents(WiFiClass& WiFi) {
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(WiFiApStarted, SYSTEM_EVENT_AP_START);
  WiFi.onEvent(WiFiApStopped, SYSTEM_EVENT_AP_STOP);  
}
