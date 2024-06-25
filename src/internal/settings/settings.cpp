#include <internal/settings/settings.h>

const char* SETTINGS_FILE = "/settings/door_settings.json";

//////////////////// Constructor
Settings::Settings(Storage *storage) {
    m_storage = storage;
    m_settingsOK = false;
}

//////////////////// Public methods implementation
bool Settings::begin() {
    if (!readSettings()) {
        // Create new empty settings file
        defaultSettings();
        if (!saveSettings())
            return false;
    }
    m_settingsOK = true;
    return true;
}

bool Settings::isSettingsOK() {
    return m_settingsOK;
}

settings_t Settings::getSettings() {
    return m_settings;
}

bool Settings::saveSettings() {
    m_storage->remove(SETTINGS_FILE);
    String json = createJson();
    if (json.equals(""))
        return false;
    
    return m_storage->writeFile(SETTINGS_FILE, json.c_str());
}

void Settings::addWifiAP(const char* ssid, const char* password) {
    wifiAP_t ap = {
        ssid: ssid,
        password: password
    };
    m_settings.wifiAPs.push_back(ap);
}

bool Settings::updWifiAP(const char* ssid, const char* password) {
    for (size_t i = 0; i < m_settings.wifiAPs.size(); i++) {
        if (m_settings.wifiAPs[i].ssid.equalsIgnoreCase(ssid)) {
            m_settings.wifiAPs[i].password = password;
            return true;
        }
    }

    return false;
}

bool Settings::delWifiAP(const char* ssid){
    uint8_t i = 0;
    while (i < m_settings.wifiAPs.size() && !m_settings.wifiAPs[i].ssid.equals(ssid)) {
        i++;
    }
    if (i >= m_settings.wifiAPs.size())
        return false;
    
    m_settings.wifiAPs.erase(m_settings.wifiAPs.begin() + i);
    return true;
}

bool Settings::ssidExists(String ssid) {
    for (size_t i = 0; i < m_settings.wifiAPs.size(); i++) {
        if (m_settings.wifiAPs[i].ssid.equalsIgnoreCase(ssid))
            return true;
    }
    return false;
}

void Settings::setMQTTValues(String server, String username, String password, uint16_t port) {
    m_settings.mqtt.server = server;
    m_settings.mqtt.username = username;
    m_settings.mqtt.password = password;
    m_settings.mqtt.port = port;
}
void Settings::setMQTTValues(String server, String username, uint16_t port) {
    m_settings.mqtt.server = server;
    m_settings.mqtt.username = username;
    m_settings.mqtt.port = port;
}
bool Settings::setMQTTCertificate(String certData) {
    if (m_settings.mqtt.caCertPath.equals(""))
        return false;

    unsigned int strLen = certData.length() + 1;
    char charData[strLen];
    certData.toCharArray(charData, strLen);
    charData[strLen] = '\0';

    m_storage->remove(m_settings.mqtt.caCertPath.c_str());
    bool ok = m_storage->writeFile(m_settings.mqtt.caCertPath.c_str(), charData);
    if (ok) {
        free(m_settings.mqtt.ca_cert);
        String cert = m_storage->readAll(m_settings.mqtt.caCertPath.c_str());
        m_settings.mqtt.ca_cert = (char*)malloc(cert.length() + 1);
        cert.toCharArray(m_settings.mqtt.ca_cert, cert.length());
        m_settings.mqtt.ca_cert[cert.length()] = '\0';
    }
    return ok;
}

void Settings::setDateValues(String server, long gmtOffset, int daylightOffset) {
    m_settings.dateTime.server = server;
    m_settings.dateTime.gmtOffset = gmtOffset;
    m_settings.dateTime.daylightOffset = daylightOffset;
}

//////////////////// Private methods implementation
bool Settings::readSettings() {
    if (!m_storage->exists(SETTINGS_FILE)) {
        return false;
    }

    StaticJsonDocument<1024> configs;
    String settingsJson = m_storage->readAll(SETTINGS_FILE);
    DeserializationError error = deserializeJson(configs, settingsJson);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    m_settings.mqtt.server = jsonObj["mqtt"]["server"].as<String>();
    m_settings.mqtt.port = jsonObj["mqtt"]["port"].as<uint16_t>();
    m_settings.mqtt.username = jsonObj["mqtt"]["username"].as<String>();
    m_settings.mqtt.password = jsonObj["mqtt"]["password"].as<String>();
    m_settings.mqtt.caCertPath = jsonObj["mqtt"]["crt_path"].as<String>();
    String cert = m_storage->readAll(m_settings.mqtt.caCertPath.c_str());
    m_settings.mqtt.ca_cert = (char*)malloc(cert.length() + 1);
    cert.toCharArray(m_settings.mqtt.ca_cert, cert.length());
    m_settings.mqtt.ca_cert[cert.length()] = '\0';

    m_settings.wifiAPs.clear();
    for (size_t i = 0; i < jsonObj["wifi"].size(); i++) {
        wifiAP_t wifi;
        wifi.ssid = jsonObj["wifi"][i]["ssid"].as<String>();
        wifi.password = jsonObj["wifi"][i]["password"].as<String>();

        m_settings.wifiAPs.push_back(wifi);
    }

    m_settings.logger.outputPath = jsonObj["data_logger"]["output_path"].as<String>();

    m_settings.dateTime.server = jsonObj["date_time"]["server"].as<String>();
    m_settings.dateTime.gmtOffset = jsonObj["date_time"]["gmt_offset"].as<long>();
    m_settings.dateTime.daylightOffset = jsonObj["date_time"]["daylight_offset"].as<int>();

    return true;
}

String Settings::createJson() {
    StaticJsonDocument<1024> doc;

    JsonObject mqttObj = doc.createNestedObject("mqtt");
    mqttObj["server"] = m_settings.mqtt.server;
    mqttObj["port"] = m_settings.mqtt.port;
    mqttObj["username"] = m_settings.mqtt.username;
    mqttObj["password"] = m_settings.mqtt.password;
    mqttObj["crt_path"] = m_settings.mqtt.caCertPath;

    JsonArray wifiArr = doc.createNestedArray("wifi");
    for (size_t i = 0; i < m_settings.wifiAPs.size(); i++) {
        JsonObject wifiAP = wifiArr.createNestedObject();
        wifiAP["ssid"] = m_settings.wifiAPs[i].ssid;
        wifiAP["password"] = m_settings.wifiAPs[i].password;
    }

    JsonObject loggerObj = doc.createNestedObject("data_logger");
    loggerObj["output_path"] = m_settings.logger.outputPath;

    JsonObject dateTimeObj = doc.createNestedObject("date_time");
    dateTimeObj["server"] = m_settings.dateTime.server;
    dateTimeObj["gmt_offset"] = m_settings.dateTime.gmtOffset;
    dateTimeObj["daylight_offset"] = m_settings.dateTime.daylightOffset;

    String json;
    serializeJsonPretty(doc, json);

    return json;
}

void Settings::defaultSettings() {
    m_settings.mqtt.server = "";
    m_settings.mqtt.port = 0;
    m_settings.mqtt.username = "";
    m_settings.mqtt.password = "";
    m_settings.mqtt.caCertPath = "/settings/mqtt_ca_root.crt";

    m_settings.wifiAPs.clear();

    m_settings.logger.outputPath = "/logs/door_data.txt";

    m_settings.dateTime.server = "pool.ntp.org";
    m_settings.dateTime.gmtOffset = -3;
    m_settings.dateTime.daylightOffset = 0;
}
