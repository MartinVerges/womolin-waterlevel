/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include "MQTTclient.h"

bool enableMqtt = false;                    // Enable Mqtt, disable to reduce power consumtion, stored in NVS

MQTTclient::MQTTclient() {}
MQTTclient::~MQTTclient() {}

void MQTTclient::addPreferences(Preferences * preferences) {
  _pref = *preferences;
}

bool MQTTclient::isConnected() {
  return client.connected();
}
bool MQTTclient::isReady() {
  if (mqttTopic.length() > 0 && isConnected()) return true;
  else return false;
}
void MQTTclient::prepare() {
  mqttTopic = _pref.getString("mqttTopic", "verges/tanklevel");
  mqttUser = _pref.getString("mqttUser", "");
  mqttPass = _pref.getString("mqttPass", "");

  if (mqttUser.length() > 0 && mqttPass.length() > 0) {
      Serial.print(F("[MQTT] Configured broker user: "));
      Serial.println(mqttUser);
      Serial.println(F("[MQTT] Configured broker pass: **hidden**"));
      client.setCredentials(mqttUser.c_str(), mqttPass.c_str());
  } else Serial.println(F("[MQTT] Configured broker without user and password!"));

  mqttHost = _pref.getString("mqttHost", "localhost");
  mqttPort = _pref.getUInt("mqttPort", 1883);

  if (mqttPort == 0 || mqttPort < 0 || mqttPort > 65535) mqttPort = 1883;
  Serial.print(F("[MQTT] Configured broker port: "));
  Serial.println(mqttPort);

  IPAddress ip;
  if (ip.fromString(mqttHost)) { // this is a valid IP
    Serial.print(F("[MQTT] Configured broker IP: "));
    Serial.println(ip);
    client.setServer(ip, mqttPort);
  } else {
    Serial.print(F("[MQTT] Configured broker host: "));
    Serial.println(mqttHost);
    client.setServer(mqttHost.c_str(), mqttPort);
  }
}

void MQTTclient::registerEvents() {
  client.onDisconnect(onMqttDisconnect);
  client.onMessage(onMqttMessage);
}

void MQTTclient::connect() {
  if (!enableMqtt) {
    Serial.println(F("[MQTT] disabled!"));
  } else {
    Serial.println(F("[MQTT] Connecting to MQTT..."));
    prepare();
    client.connect();
  }
}
void MQTTclient::disconnect(bool force) {
  client.disconnect(force);
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

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}