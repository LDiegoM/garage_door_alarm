#include <internal/net/mqtt_handlers.h>

MqttHandlers *mqtt = nullptr;

//////////////////// MQTT Handlers
void mqttMessageReceived(char* topic, uint8_t* payload, unsigned int length) {
    mqtt->processReceivedMessage(topic, payload, length);
}

//////////////////// Constructor
MqttHandlers::MqttHandlers(WiFiConnection *wifi, DoorStatus *doorStatus, Settings *settings,
           DataLogger *dataLogger, Storage *storage) {
    m_wifi = wifi;
    m_doorStatus = doorStatus;
    m_settings = settings;
    m_dataLogger = dataLogger;
    m_storage = storage;
    m_connected = false;
}

//////////////////// Public methods implementation
bool MqttHandlers::begin() {
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return false;

    m_secureClient = new WiFiClientSecure();
#ifdef ESP8266
    caCertX509 = new X509List(m_settings->getSettings().mqtt.ca_cert);
    m_secureClient->setTrustAnchors(caCertX509);
#else
    m_secureClient->setCACert(m_settings->getSettings().mqtt.ca_cert);
#endif

    m_mqttClient = new PubSubClient(*m_secureClient);
    m_mqttClient->setCallback(mqttMessageReceived);

    m_tmrConnectMQTT = new Timer(5000);

    return connect();
}

bool MqttHandlers::connect() {
    m_connected = false;
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return false;

    if (m_mqttClient->connected()) {
        m_connected = true;
        return true;
    }
    
    // If wifi is not connected, try to connect
    if (!m_wifi->isConnected()) {
        if (!m_wifi->connect())
            return false;
    }

    String clientID = "ESP32-device_" + WiFi.localIP().toString();
    m_mqttClient->setServer(m_settings->getSettings().mqtt.server.c_str(), m_settings->getSettings().mqtt.port);
    if (!m_mqttClient->connect(clientID.c_str(),
                               m_settings->getSettings().mqtt.username.c_str(),
                               m_settings->getSettings().mqtt.password.c_str())) {
        return false;
    }
    m_mqttClient->subscribe(MQTT_TOPIC_CMD);
    m_tmrConnectMQTT->stop();
    m_connected = true;

    return true;
}

bool MqttHandlers::isConnected() {
    return m_connected;
}

void MqttHandlers::loop() {
    if (!m_settings->isSettingsOK() || m_wifi->isModeAP())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;

        if (!m_tmrConnectMQTT->isRunning()) {
            m_tmrConnectMQTT->start();
        }

        if (m_tmrConnectMQTT->isTime()) {
            Serial.println("MQTT not connected. Reconnecting");
            if (!connect()) {
                return;
            }
        } else {
            return;
        }
    }

    doorStatus currentStatus = m_doorStatus->currentStatus();

    if (currentStatus != m_lastDoorStatus)
        sendValuesToMQTT(currentStatus);

    m_mqttClient->loop();
}

void MqttHandlers::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Message received from topic " + String(topic) + " - length: " + String(length));
    if (!String(topic).equals(MQTT_TOPIC_CMD))
        return;
    
    String incomingMessage = "";
    for (unsigned int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    Serial.println("incomingMessage: " + incomingMessage);

    if (incomingMessage.equals("RESEND")) {
        sendValuesToMQTT(m_doorStatus->currentStatus());
    } else if (incomingMessage.equals("GET_IP")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_IP, m_wifi->getIP().c_str(), false);
    } else if (incomingMessage.equals("GET_LOG")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_LOG, m_dataLogger->getLastLogTime().c_str(), false);
    } else if (incomingMessage.equals("GET_LOG_SIZE")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_LOGSIZE, String(m_dataLogger->logSize()).c_str(), false);
    } else if (incomingMessage.equals("GET_STO_FREE")) {
        m_mqttClient->publish(MQTT_TOPIC_RES_FREESTO, (m_storage->getFree() + " of " + m_storage->getSize()).c_str(), false);
    }
}

//////////////////// Private methods implementation
void MqttHandlers::sendValuesToMQTT(doorStatus currentStatus) {
    if (!m_settings->isSettingsOK())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;
        return;
    }
    
    String message = m_doorStatus->getDoorStatus(currentStatus);
    if (m_mqttClient->publish(MQTT_TOPIC_STATUS, message.c_str(), true))
        m_lastDoorStatus = currentStatus;
}
