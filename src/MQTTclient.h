/**
 * @file MQTTclient.h
 * @author Martin Verges <martin@verges.cc>
 * @brief MQTT client library
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/waterlevel
 * 
 * License: CC BY-NC-SA 4.0
**/

#include <Preferences.h>
#include <WiFi.h>
#include <PubSubClient.h>

extern bool enableMqtt;

class MQTTclient {
    public:
        String mqttTopic;
        String mqttUser;
        String mqttPass;
        String mqttHost;
        String mqttClientId;
        uint16_t mqttPort;

		MQTTclient();
        virtual ~MQTTclient();

        bool isConnected();
        bool isReady();
        void prepare(String host, uint16_t port, String topic, String user, String pass);
        void registerEvents();
        void connect();
        void disconnect();

        PubSubClient client;
    private:
        WiFiClient ethClient;
};
