#ifndef mqtt_connection_h
#define mqtt_connection_h

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <vector>

#include <internal/core/timer.h>
#include <internal/core/logging.h>
#include <internal/core/wifi_connection.h>
#include <internal/core/storage.h>

struct mqtt_t {
    String server, username, password, caCertPath;
    uint16_t port;
    char *ca_cert;
};

struct cmdMessage_t {
    String deviceID, cmd, value;
};

class MqttConnection {
    private:
        String m_appName;
        String m_deviceID;
        String m_location;
        WiFiConnection *m_wifi;
        Storage *m_storage;
        mqtt_t m_settings;

#ifdef ESP8266
        X509List *caCertX509;
#endif
        WiFiClientSecure *m_secureClient;
        PubSubClient *m_mqttClient;

        bool m_connected;
        Timer *m_tmrConnectMQTT;

        const char* MQTT_TOPIC_ADM_CMD      = "topic-{app_name}-adm-cmd";
        const char* MQTT_TOPIC_ADM_DEVICE   = "topic-{app_name}-adm-device";
        const char* MQTT_TOPIC_ADM_IP       = "topic-{app_name}-adm-ip";
        const char* MQTT_TOPIC_ADM_LOGSIZE  = "topic-{app_name}-adm-logsize";
        const char* MQTT_TOPIC_ADM_FREESTO  = "topic-{app_name}-adm-freesto";
        const char* MQTT_TOPIC_ADM_LOCATION = "topic-{app_name}-adm-location";

        std::vector<String> m_subcriptionTopics;

        String getTopicName(const char* topicName);

        // processJsonMessage parse a json received message. It returns the effective command, only
        // if asked device_id is current device. Message format:
        // {"cmd": string, "device_id": string}
        String processJsonMessage(String message);

        // encodeMessageJson encodes a data into a json message to sent to a specific topic. Format:
        // {"device_id": string, "value": string}
        String encodeMessageJson(String data);

    public:
        MqttConnection(String appName, String deviceID, String location, WiFiConnection *wifi, Storage *storage, mqtt_t settings);

        void setDeviceID(String deviceID);
        void setLocation(String location);
        bool begin();
        bool connect();
        bool isConnected();
        void loop();
        void processReceivedMessage(char* topic, uint8_t* payload, unsigned int length);
        void setCallback(MQTT_CALLBACK_SIGNATURE);
        bool subscribe(const char* topic);
        bool publish(const char* topic, const char* payload);
        bool publish(const char* topic, const char* payload, boolean retained);
};

extern MqttConnection *_mqtt;

#endif
