/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include "mqtt.h"

AsyncMqttClient mqttClient;

bool enableMqtt = false;                    // Enable Mqtt, disable to reduce power consumtion, stored in NVS
String mqttTopic = "";                      // Base name of the MQTT Topic

void prepareMqtt(Preferences& preferences) {
  mqttClient.onDisconnect(onMqttDisconnect);

  mqttTopic = preferences.getString("mqttTopic", "verges/tanklevel");

  String mqttUser = preferences.getString("mqttUser", "");
  String mqttPass = preferences.getString("mqttPass", "");
  if (mqttUser.length() > 0 && mqttPass.length() > 0) {
    Serial.print(F("[MQTT] Configured broker user: "));
    Serial.println(mqttUser);
    Serial.println(F("[MQTT] Configured broker pass: **hidden**"));
    mqttClient.setCredentials(mqttUser.c_str(), mqttPass.c_str());
  } else Serial.println(F("[MQTT] Configured broker without user and password!"));

  String mqttHost = preferences.getString("mqttHost", "localhost");
  uint16_t mqttPort = preferences.getUInt("mqttPort", 1883);
  if (mqttPort == 0) mqttPort = 1883;
  Serial.print(F("[MQTT] Configured broker port: "));
  Serial.println(mqttPort);

  Serial.print(F("[MQTT] Configured broker host: "));
  IPAddress ip;
  if (ip.fromString(mqttHost)) { // this is a valid IP
    Serial.println(ip);
    mqttClient.setServer(ip, mqttPort);
  } else {
    Serial.println(mqttHost);
    mqttClient.setServer(mqttHost.c_str(), mqttPort);
  }
}

void connectToMqtt() {
  if (!enableMqtt) {
    Serial.println(F("[MQTT] disabled!"));
  } else {
    Serial.println(F("[MQTT] Connecting to MQTT..."));
    mqttClient.connect();
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.print(F("[MQTT] Disconnected from MQTT with reason: "));
  Serial.println(static_cast<uint8_t>(reason));

  if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
    Serial.println(F("[MQTT] ==> TCP disconnected."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
    Serial.println(F("[MQTT] ==> Unacceptable protocol version."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
    Serial.println(F("[MQTT] ==> Identifier rejected."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
    Serial.println(F("[MQTT] ==> The server is unavailable."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
    Serial.println(F("[MQTT] ==> Malformed credentials."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
    Serial.println(F("[MQTT] ==> Not authorized, credentials required."));
  } else if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
    Serial.println(F("[MQTT] ==> TLS bad fingerprint."));
  }
}
