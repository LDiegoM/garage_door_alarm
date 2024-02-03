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
#include <internal/low_level/data_logger.h>

#include <internal/door_device/door_status.h>

//////////////////// MQTT Handlers
void mqttMessageReceived(char* topic, uint8_t* payload, unsigned int length);

class MqttHandlers : public CommonMqttHandlers {
    private:
        WiFiConnection *m_wifi;
        DoorStatus *m_doorStatus;
        Settings *m_settings;
        DataLogger *m_dataLogger;
        Storage *m_storage;

#ifdef ESP8266
        X509List *caCertX509;
#endif
        WiFiClientSecure *m_secureClient;

        PubSubClient *m_mqttClient;

        bool m_connected;
        Timer *m_tmrConnectMQTT;
        const char* MQTT_TOPIC_GARAGE_DOOR = "topic-garage-door";
        const char* MQTT_TOPIC_CMD = "topic-door-cmd";
        const char* MQTT_TOPIC_RES_IP = "topic-door-res-ip";
        const char* MQTT_TOPIC_RES_LOG = "topic-door-res-log";
        const char* MQTT_TOPIC_RES_LOGSIZE = "topic-door-res-logsize";
        const char* MQTT_TOPIC_RES_FREESTO = "topic-door-res-freesto";

        doorStatus m_lastDoorStatus = Unknown;

        void sendDoorStatusToMQTT(doorStatus currentStatus);

    public:
        MqttHandlers(WiFiConnection *wifi, DoorStatus *doorStatus, Settings *settings,
             DataLogger *dataLogger, Storage *storage);
        
        bool begin();
        bool connect();
        bool isConnected();
        void loop();
        void processReceivedMessage(char* topic, uint8_t* payload, unsigned int length);
        bool publish(const char* topic, const char* payload);
        bool publish(const char* topic, const char* payload, boolean retained);
};

extern MqttHandlers *mqtt;

#endif
