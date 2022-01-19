/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <WiFi.h>

bool enableWifi = true;                     // Enable Wifi, disable to reduce power consumtion, stored in NVS

void MDNSBegin(String hostname);
void MDNSEnd();

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStarted(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStopped(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiRegisterEvents(WiFiClass& WiFi);
