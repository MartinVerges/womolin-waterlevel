/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <AsyncMqttClient.h>

extern bool enableMqtt;

void connectToMqtt();
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttConnect(bool sessionPresent);
