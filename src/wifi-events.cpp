/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <WiFi.h>
#include "mqtt.h"

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("[WIFI] WiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
  connectToMqtt();
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] Connected successfully!");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("[WIFI] Disconnected from WiFi access point with Reason:");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.reconnect();
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
