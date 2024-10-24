#include <handlers/mqtt.h>

MqttHandlers *mqttHandlers = nullptr;

//////////////////// Constructor
MqttHandlers::MqttHandlers(CommonDoorStatus *doorStatus) {
    m_doorStatus = doorStatus;
    m_tmrSendMessage = new Timer(1000);
}

//////////////////// Public methods implementation
void MqttHandlers::begin() {
    if (_mqtt == nullptr || !_mqtt->isConnected())
        return;

    lg->debug("MqttHandlers::begin() - _mqtt is connected (WTF!)", __FILE__, __LINE__);
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

    if (currentStatus != m_lastDoorStatus && (!m_tmrSendMessage->isRunning() || m_tmrSendMessage->isTime())) {
        lg->info("Door status just changed and was not sent to MQTT yet", __FILE__, __LINE__,
            lg->newTags()->add("status", m_doorStatus->getDoorStatus(currentStatus))
        );
        if (sendDoorStatusToMQTT(currentStatus)) {
            m_tmrSendMessage->stop();
        } else {
            m_tmrSendMessage->start();
        }
    }

    _mqtt->loop();
}

//////////////////// Private methods implementation
bool MqttHandlers::sendDoorStatusToMQTT(doorStatus currentStatus) {
    if (_mqtt == nullptr || !_mqtt->isConnected())
        return false;

    String message = "CLOSE";
    if (currentStatus == Opened)
        message = "OPEN";

    bool sent = _mqtt->publish(MQTT_TOPIC_GARAGE_DOOR, message.c_str(), true);
    if (sent)
        m_lastDoorStatus = currentStatus;
    
    return sent;
}
