#include <internal/core/mqtt_connection.h>

MqttConnection *_mqtt = nullptr;

//////////////////// Constructor
MqttConnection::MqttConnection(String appName, String deviceID, String location, WiFiConnection *wifi, Storage *storage, mqtt_t settings) {
    m_appName = appName;
    m_deviceID = deviceID;
    m_location = location;
    m_wifi = wifi;
    m_storage = storage;
    m_settings = settings;
}

//////////////////// Public methods implementation
void MqttConnection::setDeviceID(String deviceID) {
    m_deviceID = deviceID;
}
void MqttConnection::setLocation(String location) {
    m_location = location;
}

bool MqttConnection::begin() {
    if (m_settings.server.equals("") || m_wifi->isModeAP())
        return false;

    m_secureClient = new WiFiClientSecure();
#ifdef ESP8266
    caCertX509 = new X509List(m_settings.ca_cert);
    m_secureClient->setTrustAnchors(caCertX509);
#else
    m_secureClient->setCACert(m_settings.ca_cert);
#endif

    m_mqttClient = new PubSubClient(*m_secureClient);
    m_mqttClient->setCallback([](char* topic, uint8_t* payload, unsigned int length){
        _mqtt->processReceivedMessage(topic, payload, length);
    });

    m_tmrConnectMQTT = new Timer(5000);

    return connect();
}

bool MqttConnection::connect() {
    m_connected = false;
    if (m_settings.server.equals("") || m_wifi->isModeAP())
        return false;

    if (m_mqttClient->connected()) {
        m_connected = true;
        return true;
    }
    
    // If wifi is not connected, try to connect
    if (!m_wifi->isConnected()) {
        if (!m_wifi->connect(true))
            return false;
    }

    String clientID = m_deviceID + m_wifi->getIP();
    m_mqttClient->setServer(m_settings.server.c_str(), m_settings.port);
    if (!m_mqttClient->connect(clientID.c_str(),
                               m_settings.username.c_str(),
                               m_settings.password.c_str())) {
        lg->warn("Fail to connect to mqtt service", __FILE__, __LINE__);
        return false;
    }
    String cmdTopic = getTopicName(MQTT_TOPIC_ADM_CMD);
    lg->debug("Device connected to mqtt. Subscribing to cmd topic", __FILE__, __LINE__,
        lg->newTags()->add("cmd_topic", cmdTopic)
    );
    m_mqttClient->subscribe(cmdTopic.c_str());

    for (size_t i = 0; i < m_subcriptionTopics.size(); i++) {
        lg->debug("Device connected to mqtt. Subscribing to external topic", __FILE__, __LINE__,
            lg->newTags()->add("external_topic", m_subcriptionTopics[i])
        );
        m_mqttClient->subscribe(m_subcriptionTopics[i].c_str());
    }

    m_tmrConnectMQTT->stop();
    m_connected = true;

    return true;
}

bool MqttConnection::isConnected() {
    return m_connected;
}

void MqttConnection::loop() {
    if (m_settings.server.equals("") || m_wifi->isModeAP())
        return;

    if (!m_mqttClient->connected()) {
        m_connected = false;

        if (!m_tmrConnectMQTT->isRunning()) {
            m_tmrConnectMQTT->start();
        }

        if (m_tmrConnectMQTT->isTime()) {
            lg->warn("MQTT not connected. Reconnecting", __FILE__, __LINE__);
            if (!connect()) {
                return;
            }
        } else {
            return;
        }
    }

    m_mqttClient->loop();
}

void MqttConnection::processReceivedMessage(char* topic, uint8_t* payload, unsigned int length) {
    String sTopic = String(topic);
    lg->debug("Message received from topic", __FILE__, __LINE__,
        lg->newTags()
            ->add("topic", sTopic)
            ->add("length", String(length))
    );
    if (!getTopicName(MQTT_TOPIC_ADM_CMD).equals(sTopic)) {
        lg->debug("Message received from another topic", __FILE__, __LINE__);
        return;
    }

    lg->debug("Message received from command topic. Composing incoming message.", __FILE__, __LINE__);
    String incomingMessage = "";
    for (unsigned int i = 0; i < length; i++)
        incomingMessage += (char)payload[i];
    
    lg->debug("incomingMessage from topic", __FILE__, __LINE__,
        lg->newTags()->add("message", incomingMessage)
    );

    if (incomingMessage.equals("DEVICE_ID")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_DEVICE).c_str(), m_deviceID.c_str(), false);
        return;
    } else if (incomingMessage.equals("GET_LOCATION")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_LOCATION).c_str(), encodeMessageJson(m_location).c_str(), false);
        return;
    } else if (incomingMessage.equals("GET_IP")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_IP).c_str(), encodeMessageJson(m_wifi->getIP()).c_str(), false);
        return;
    }
     
    // Parse incoming message as json
    String cmd = processJsonMessage(incomingMessage);
    if (cmd.equals("GET_IP")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_IP).c_str(), encodeMessageJson(m_wifi->getIP()).c_str(), false);
    } else if (cmd.equals("GET_LOG_SIZE")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_LOGSIZE).c_str(), encodeMessageJson(String(lg->logSize())).c_str(), false);
    } else if (cmd.equals("GET_STO_FREE")) {
        m_mqttClient->publish(
            getTopicName(MQTT_TOPIC_ADM_FREESTO).c_str(),
            encodeMessageJson(m_storage->getFree() + " of " + m_storage->getSize()).c_str(), false
        );
    } else if (cmd.equals("GET_LOCATION")) {
        m_mqttClient->publish(getTopicName(MQTT_TOPIC_ADM_LOCATION).c_str(), encodeMessageJson(m_location).c_str(), false);
    }
}

void MqttConnection::setCallback(MQTT_CALLBACK_SIGNATURE) {
    m_mqttClient->setCallback(callback);
}

bool MqttConnection::subscribe(const char* topic) {
    m_subcriptionTopics.push_back(String(topic));
    return m_mqttClient->subscribe(topic);
}

bool MqttConnection::publish(const char* topic, const char* payload){
    return publish(topic, payload, false);
}
bool MqttConnection::publish(const char* topic, const char* payload, boolean retained){
    return m_mqttClient->publish(topic, payload, retained);
}

//////////////////// Private methods implementation
String MqttConnection::processJsonMessage(String message) {
    cmdMessage_t cmd;

    StaticJsonDocument<256> cmdJson;
    DeserializationError error = deserializeJson(cmdJson, message);
    if (error) {
        lg->error("fail to parse received message", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return "";
    }
    JsonObject jsonObj = cmdJson.as<JsonObject>();

    cmd.deviceID = jsonObj["device_id"].as<String>();
    cmd.cmd = jsonObj["cmd"].as<String>();
    if (cmd.deviceID.equals(m_deviceID))
        return cmd.cmd;
    
    return "";
}

String MqttConnection::encodeMessageJson(String data) {
    String j = "{";
    j += "\"device_id\":\"" + m_deviceID + "\",";
    j += "\"value\":\"" + data + "\"";
    j += "}";
    return j;
}

String MqttConnection::getTopicName(const char* topicName) {
    String s = String(topicName);
    s.replace("{app_name}", m_appName);
    return s;
}
