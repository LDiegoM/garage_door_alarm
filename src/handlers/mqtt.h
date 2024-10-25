#ifndef handlers_mqtt_h
#define handlers_mqtt_h

#include <internal/core/mqtt_connection.h>
#include <internal/common/door_status.h>

struct handlers_mqtt_t {
    mqtt_t connection;
};

class MqttHandlers {
    private:
        const char* MQTT_TOPIC_GARAGE_DOOR = "topic-garage-door";
        const char* MQTT_TOPIC_GARAGE_DOOR_CMD = "topic-garage-door-cmd";

        CommonDoorStatus *m_doorStatus;
        doorStatus m_lastDoorStatus = Unknown;
        Timer *m_tmrSendMessage;

        bool sendDoorStatusToMQTT(doorStatus currentStatus);

    public:
        MqttHandlers(CommonDoorStatus *doorStatus);

        void begin();
        void processReceivedMessage(String &topic, String &payload);
        void loop();
};

extern MqttHandlers *mqttHandlers;

#endif
