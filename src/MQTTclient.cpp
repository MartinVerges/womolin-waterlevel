/**
 * @file MQTTclient.cpp
 * @author Martin Verges <martin@verges.cc>
 * @brief MQTT client library
 * @version 0.1
 * @date 2022-05-29
**/

#include "log.h"

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
      LOG_INFO(F("[MQTT] Configured broker user: "));
      LOG_INFO_LN(mqttUser);
      LOG_INFO_LN(F("[MQTT] Configured broker pass: **hidden**"));
  } else LOG_INFO_LN(F("[MQTT] Configured broker without user and password!"));

  if (mqttPort == 0 || mqttPort < 0 || mqttPort > 65535) mqttPort = 1883;
  LOG_INFO(F("[MQTT] Configured broker port: "));
  LOG_INFO_LN(mqttPort);

  IPAddress ip;
  if (ip.fromString(mqttHost)) { // this is a valid IP
    LOG_INFO(F("[MQTT] Configured broker IP: "));
    LOG_INFO_LN(ip);
    client.setServer(ip, mqttPort);
  } else {
    LOG_INFO(F("[MQTT] Configured broker host: "));
    LOG_INFO_LN(mqttHost);
    client.setServer(mqttHost.c_str(), mqttPort);
  }
}

void MQTTclient::connect() {
  if (!enableMqtt) {
    LOG_INFO_LN(F("[MQTT] disabled!"));
  } else {
    LOG_INFO_LN(F("[MQTT] Connecting to MQTT..."));
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

    switch (client.state()) {
    case MQTT_CONNECTION_TIMEOUT:
      LOG_INFO_LN(F("[MQTT] ... connection time out"));
      break;
    case MQTT_CONNECTION_LOST:
      LOG_INFO_LN(F("[MQTT] ... connection lost"));
      break;
    case MQTT_CONNECT_FAILED:
      LOG_INFO_LN(F("[MQTT] ... connection failed"));
      break;
    case MQTT_DISCONNECTED:
      LOG_INFO_LN(F("[MQTT] ... disconnected"));
      break;
    case MQTT_CONNECTED:
      LOG_INFO_LN(F("[MQTT] ... connected"));
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      LOG_INFO_LN(F("[MQTT] ... connection error: bad protocol"));
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      LOG_INFO_LN(F("[MQTT] ... connection error: bad client ID"));
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      LOG_INFO_LN(F("[MQTT] ... connection error: unavailable"));
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      LOG_INFO_LN(F("[MQTT] ... connection error: bad credentials"));
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      LOG_INFO_LN(F("[MQTT] ... connection error: unauthorized"));
      break;
    default:
      LOG_INFO(F("[MQTT] ... connection error: unknown code "));
      LOG_INFO_LN(client.state());
      break;
    }
  }
}
void MQTTclient::disconnect() {
  client.disconnect();
}

/*
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  LOG_INFO(F("[MQTT] Disconnected from MQTT with reason: "));
  LOG_INFO_LN(static_cast<uint8_t>(reason));

  if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
    LOG_INFO_LN(F("[MQTT] ==> TCP disconnected."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
    LOG_INFO_LN(F("[MQTT] ==> Unacceptable protocol version."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
    LOG_INFO_LN(F("[MQTT] ==> Identifier rejected."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
    LOG_INFO_LN(F("[MQTT] ==> The server is unavailable."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
    LOG_INFO_LN(F("[MQTT] ==> Malformed credentials."));
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
    LOG_INFO_LN(F("[MQTT] ==> Not authorized, credentials required."));
  } else if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
    LOG_INFO_LN(F("[MQTT] ==> TLS bad fingerprint."));
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  LOG_INFO_LN("Publish received.");
  LOG_INFO("  topic: ");
  LOG_INFO_LN(topic);
  LOG_INFO("  qos: ");
  LOG_INFO_LN(properties.qos);
  LOG_INFO("  dup: ");
  LOG_INFO_LN(properties.dup);
  LOG_INFO("  retain: ");
  LOG_INFO_LN(properties.retain);
  LOG_INFO("  len: ");
  LOG_INFO_LN(len);
  LOG_INFO("  index: ");
  LOG_INFO_LN(index);
  LOG_INFO("  total: ");
  LOG_INFO_LN(total);
}

17:37:08.132 > [OTA] Password set to '3399731656'
17:37:08.143 > [MQTT] Connecting to MQTT...
17:37:08.157 > 0
17:37:11.168 > Guru Meditation Error: Core  1 panic'ed (IntegerDivideByZero). Exception was unhandled.
17:37:11.176 > 
17:37:11.176 > Core  1 register dump:
17:37:11.178 > PC      : 0x400f76a0  PS      : 0x00060630  A0      : 0x800f7948  A1      : 0x3ffd0870  
17:37:11.186 > A2      : 0x00000000  A3      : 0x00000000  A4      : 0x00000000  A5      : 0x3ffc6100  
17:37:11.194 > A6      : 0x00000004  A7      : 0x00000020  A8      : 0x00000000  A9      : 0x3ffc4338  
17:37:11.201 > A10     : 0x000000f8  A11     : 0x3ffd086d  A12     : 0x3ffca00c  A13     : 0x00000002  
17:37:11.209 > A14     : 0x3ffd086c  A15     : 0xff000000  SAR     : 0x00000008  EXCCAUSE: 0x00000006  
17:37:11.217 > EXCVADDR: 0x00000000  LBEG    : 0x40091720  LEND    : 0x40091736  LCOUNT  : 0xffffffff  
17:37:11.225 > 
17:37:11.225 > 
17:37:11.225 > Backtrace:0x400f769d:0x3ffd08700x400f7945:0x3ffd0890 0x400f7a22:0x3ffd08c0 0x400dd9f1:0x3ffd08f0 0x401151a9:0x3ffd0e30 
17:37:11.279 >   #0  0x400f769d:0x3ffd08700 in PubSubClient::connect(char const*, char const*, char const*, char const*, unsigned char, bool, char const*, bool) at .pio/libdeps/esp32dev/PubSubClient/src/PubSubClient.cpp:245
*/