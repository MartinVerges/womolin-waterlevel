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

void connectToMqtt() {
  if (!enableMqtt) {
    Serial.println(F("[MQTT] disabled"));
  } else {
    Serial.println(F("[MQTT] Connecting to MQTT..."));
    mqttClient.connect();
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println(F("[MQTT] Disconnected from MQTT."));
}

void onMqttConnect(bool sessionPresent) {
  Serial.println(F("Connected to MQTT."));
  Serial.print(F("Session present: "));
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("test/lol", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}
