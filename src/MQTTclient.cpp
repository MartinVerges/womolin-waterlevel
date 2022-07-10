/**
 * @file MQTTclient.cpp
 * @author Martin Verges <martin@verges.cc>
 * @brief MQTT client library
 * @version 0.1
 * @date 2022-05-29
**/

#include "MQTTclient.h"

bool enableMqtt = false;                    // Enable Mqtt, disable to reduce power consumtion, stored in NVS

MQTTclient::MQTTclient() {
  client.setClient(ethClient);
}
MQTTclient::~MQTTclient() {}

bool MQTTclient::isConnected() {
  return client.connected();
}

bool MQTTclient::isReady() {
  if (mqttTopic.length() > 0 && isConnected()) return true;
  else return false;
}

void MQTTclient::prepare(String host, uint16_t port, String topic, String user, String pass) {
  mqttHost = host;
  mqttPort = port;
  mqttTopic = topic;
  mqttUser = user;
  mqttPass = pass;

  // username+password will be used on connect()
  if (mqttUser.length() > 0 && mqttPass.length() > 0) {
      Serial.print(F("[MQTT] Configured broker user: "));
      Serial.println(mqttUser);
      Serial.println(F("[MQTT] Configured broker pass: **hidden**"));
  } else Serial.println(F("[MQTT] Configured broker without user and password!"));

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
//  client.onDisconnect(onMqttDisconnect);
//  client.onMessage(onMqttMessage);
}

void MQTTclient::connect() {
  if (!enableMqtt) {
    Serial.println(F("[MQTT] disabled!"));
  } else {
    Serial.println(F("[MQTT] Connecting to MQTT..."));
    client.connect(
      mqttClientId.c_str(),
      mqttUser.length() > 0 ? mqttUser.c_str() : NULL,
      mqttPass.length() > 0 ? mqttPass.c_str() : NULL,
      0,
      0,
      1,
      0,
      1
    );
    sleep(0.5);
    Serial.println(client.state());
  }
}
void MQTTclient::disconnect() {
  client.disconnect();
}

/*
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
*/