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

class MQTTclient {
    private:
        Preferences _pref;
    public:
        String mqttTopic;
        String mqttUser;
        String mqttPass;
        String mqttHost;
        uint16_t mqttPort;

		MQTTclient();
        virtual ~MQTTclient();

        void addPreferences(Preferences * preferences);
        bool isConnected();
        bool isReady();
        void prepare();
        void registerEvents();
        void connect();
        void disconnect(bool force = false);

        AsyncMqttClient client;
};

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
