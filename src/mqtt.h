/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <AsyncMqttClient.h>
#include <Preferences.h>

extern bool enableMqtt;

void prepareMqtt(Preferences& preferences);
void connectToMqtt();
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
