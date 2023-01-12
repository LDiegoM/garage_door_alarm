/*
    This is an implementation of MQTT queues for gatage door alarm.

    Diego M. Lopez - 2021 (ldiegom@gmail.com)
*/

#ifndef mqtt_handlers_h
#define mqtt_handlers_h

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <internal/common/mqtt_handlers.h>
#include <internal/low_level/storage.h>
#include <internal/low_level/timer.h>
#include <internal/net/wifi_connection.h>
#include <internal/settings/settings.h>

//////////////////// MQTT Handlers
void mqttMessageReceived(char* topic, uint8_t* payload, unsigned int length);

class AlarmMqttHandlers : public CommonMqttHandlers {
    private:
        WiFiConnection *m_wifi;
        Settings *m_settings;
        Storage *m_storage;

#ifdef ESP8266
        X509List *caCertX509;
#endif
        WiFiClientSecure *m_secureClient;

        PubSubClient *m_mqttClient;

        bool m_connected;
        Timer *m_tmrConnectMQTT;
        const char* MQTT_TOPIC_STATUS = "topic-door-status";
        const char* MQTT_TOPIC_CMD = "topic-alarm-cmd";
        const char* MQTT_TOPIC_RES_IP = "topic-alarm-res-ip";
        const char* MQTT_TOPIC_RES_FREESTO = "topic-alarm-res-freesto";

        String m_currentDoorStatus = "";

    public:
        AlarmMqttHandlers(WiFiConnection *wifi, Settings *settings, Storage *storage);
        
        bool begin();
        bool connect();
        bool isConnected();
        void loop();
        void processReceivedMessage(char* topic, uint8_t* payload, unsigned int length);
};

extern AlarmMqttHandlers *alarmMqtt;

#endif
