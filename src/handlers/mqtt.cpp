#include <handlers/mqtt.h>

MqttHandlers *mqttHandlers = nullptr;

//////////////////// Constructor
MqttHandlers::MqttHandlers(CommonDoorStatus *doorStatus) {
    m_doorStatus = doorStatus;
}

//////////////////// Public methods implementation
void MqttHandlers::begin() {
    if (_mqtt == nullptr)
        return;

    _mqtt->subscribe(MQTT_TOPIC_GARAGE_DOOR_CMD);

    lg->debug("mqtt_handlers.begin() - registering handler callback", __FILE__, __LINE__);
    _mqtt->setCallback([](char* topic, uint8_t* payload, unsigned int length){
        mqttHandlers->processReceivedMessage(topic, payload, length);
    });
}

void MqttHandlers::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    if (_mqtt == nullptr)
        return;

    String sTopic = String(topic);

    _mqtt->processReceivedMessage(topic, payload, length);

    if (!sTopic.equals(MQTT_TOPIC_GARAGE_DOOR_CMD)) {
        return;
    }

    lg->debug("Message received from garage door topic. Composing incoming message.", __FILE__, __LINE__);
    String incomingMessage = "";
    for (unsigned int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    lg->debug("incomingMessage from topic", __FILE__, __LINE__,
        lg->newTags()->add("message", incomingMessage)
    );

    if (incomingMessage.equals("RESEND")) {
        sendDoorStatusToMQTT(m_doorStatus->currentStatus());
    }
}

void MqttHandlers::loop() {
    if (_mqtt == nullptr)
        return;

    doorStatus currentStatus = m_doorStatus->currentStatus();

    if (currentStatus != m_lastDoorStatus) {
        lg->info("Door status just changed and was not sent to MQTT yet", __FILE__, __LINE__,
            lg->newTags()->add("status", m_doorStatus->getDoorStatus(currentStatus))
        );
        sendDoorStatusToMQTT(currentStatus);
    }

    _mqtt->loop();
}

//////////////////// Private methods implementation
void MqttHandlers::sendDoorStatusToMQTT(doorStatus currentStatus) {
    if (_mqtt == nullptr || !_mqtt->isConnected())
        return;

    String message = "CLOSE";
    if (currentStatus == Opened)
        message = "OPEN";

    if (_mqtt->publish(MQTT_TOPIC_GARAGE_DOOR, message.c_str(), true))
        m_lastDoorStatus = currentStatus;
}
