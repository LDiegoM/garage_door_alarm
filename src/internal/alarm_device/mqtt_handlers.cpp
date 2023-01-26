#include <internal/alarm_device/mqtt_handlers.h>

AlarmMqttHandlers *alarmMqtt = nullptr;

//////////////////// MQTT Handlers
void alarmMqttMessageReceived(char* topic, uint8_t* payload, unsigned int length) {
    alarmMqtt->processReceivedMessage(topic, payload, length);
}

//////////////////// Constructor
AlarmMqttHandlers::AlarmMqttHandlers(WiFiConnection *wifi, Settings *settings, Storage *storage) {
    m_wifi = wifi;
    m_settings = settings;
    m_storage = storage;
    m_connected = false;
}

//////////////////// Public methods implementation
bool AlarmMqttHandlers::begin() {
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
    m_mqttClient->setCallback(alarmMqttMessageReceived);

    m_tmrConnectMQTT = new Timer(5000);

    return connect();
}

bool AlarmMqttHandlers::connect() {
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
    m_mqttClient->subscribe(MQTT_TOPIC_STATUS);
    m_mqttClient->subscribe(MQTT_TOPIC_CMD);
    m_tmrConnectMQTT->stop();
    m_connected = true;

    return true;
}

bool AlarmMqttHandlers::isConnected() {
    return m_connected;
}

void AlarmMqttHandlers::loop() {
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

    m_mqttClient->loop();
}

void AlarmMqttHandlers::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Message received from topic " + String(topic) + " - length: " + String(length));
    if (!String(topic).equals(MQTT_TOPIC_CMD) && !String(topic).equals(MQTT_TOPIC_STATUS))
        return;
    
    String incomingMessage = "";
    for (unsigned int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    Serial.println("incomingMessage: " + incomingMessage);

    if (String(topic).equals(MQTT_TOPIC_CMD)) {
        if (incomingMessage.equals("GET_IP")) {
            m_mqttClient->publish(MQTT_TOPIC_RES_IP, m_wifi->getIP().c_str(), false);
        } else if (incomingMessage.equals("GET_STO_FREE")) {
            m_mqttClient->publish(MQTT_TOPIC_RES_FREESTO, (m_storage->getFree() + " of " + m_storage->getSize()).c_str(), false);
        }
    } else {
        m_currentDoorStatus = incomingMessage;
    }
}

//////////////////// Private methods implementation
