#include <internal/settings/settings.h>

const char* SETTINGS_FILE = "/settings/door_settings.json";

//////////////////// Constructor
Settings::Settings(Application *app) {
    m_app = app;
    m_storage = app->storage();
    m_settingsOK = false;
}

//////////////////// Public methods implementation
bool Settings::begin() {
    if (!readSettings()) {
        // Create new empty settings file
        defaultSettings();
        if (!saveSettings()) {
            lg->error("fail to save defaulted settings error", __FILE__, __LINE__);
            return false;
        }

        lg->warn("Settings were defaulted. Must config", __FILE__, __LINE__);
    } else {
        lg->info("Settings are OK", __FILE__, __LINE__);
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

void Settings::setDeviceValue(String deviceID) {
    m_settings.app.deviceID = deviceID;
}
void Settings::setDeviceValue(String deviceID, String geoLocationLat, String geoLocationLng) {
    m_settings.app.deviceID = deviceID;
    m_settings.app.geoLocation.lat = geoLocationLat;
    m_settings.app.geoLocation.lng = geoLocationLng;
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
    m_settings.mqtt.connection.server = server;
    m_settings.mqtt.connection.username = username;
    m_settings.mqtt.connection.password = password;
    m_settings.mqtt.connection.port = port;
}
void Settings::setMQTTValues(String server, String username, uint16_t port) {
    m_settings.mqtt.connection.server = server;
    m_settings.mqtt.connection.username = username;
    m_settings.mqtt.connection.port = port;
}
bool Settings::setMQTTCertificate(String certData) {
    if (m_settings.mqtt.connection.caCertPath.equals(""))
        return false;

    unsigned int strLen = certData.length() + 1;
    char charData[strLen];
    certData.toCharArray(charData, strLen);
    charData[strLen] = '\0';

    m_storage->remove(m_settings.mqtt.connection.caCertPath.c_str());
    bool ok = m_storage->writeFile(m_settings.mqtt.connection.caCertPath.c_str(), charData);
    if (ok) {
        free(m_settings.mqtt.connection.ca_cert);
        String cert = m_storage->readAll(m_settings.mqtt.connection.caCertPath.c_str());
        m_settings.mqtt.connection.ca_cert = (char*)malloc(cert.length() + 1);
        cert.toCharArray(m_settings.mqtt.connection.ca_cert, cert.length());
        m_settings.mqtt.connection.ca_cert[cert.length()] = '\0';
    }
    return ok;
}

void Settings::setLoggingValues(uint8_t level, uint16_t refreshPeriod) {
    m_settings.logging.level = level;
    m_settings.logging.refreshPeriod = refreshPeriod;

    lg->setLevel(level);
    lg->setRefreshPeriod(refreshPeriod);
}

void Settings::setDateValues(String server1, String server2, long gmtOffset, int daylightOffset) {
    m_settings.dateTime.server1 = server1;
    m_settings.dateTime.server2 = server2;
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

    m_settings.app.deviceID = jsonObj["device"]["device_id"].as<String>();
    m_settings.app.geoLocation.lat = jsonObj["device"]["geo_location_lat"].as<String>();
    m_settings.app.geoLocation.lng = jsonObj["device"]["geo_location_lng"].as<String>();

    m_settings.mqtt.connection.server = jsonObj["mqtt"]["server"].as<String>();
    m_settings.mqtt.connection.port = jsonObj["mqtt"]["port"].as<uint16_t>();
    m_settings.mqtt.connection.username = jsonObj["mqtt"]["username"].as<String>();
    m_settings.mqtt.connection.password = jsonObj["mqtt"]["password"].as<String>();
    m_settings.mqtt.connection.caCertPath = jsonObj["mqtt"]["crt_path"].as<String>();
    String cert = m_storage->readAll(m_settings.mqtt.connection.caCertPath.c_str());
    m_settings.mqtt.connection.ca_cert = (char*)malloc(cert.length() + 1);
    cert.toCharArray(m_settings.mqtt.connection.ca_cert, cert.length());
    m_settings.mqtt.connection.ca_cert[cert.length()] = '\0';

    m_settings.wifiAPs.clear();
    for (size_t i = 0; i < jsonObj["wifi"].size(); i++) {
        wifiAP_t wifi;
        wifi.ssid = jsonObj["wifi"][i]["ssid"].as<String>();
        wifi.password = jsonObj["wifi"][i]["password"].as<String>();

        m_settings.wifiAPs.push_back(wifi);
    }

    m_settings.dateTime.server1 = jsonObj["date_time"]["server1"].as<String>();
    m_settings.dateTime.server2 = jsonObj["date_time"]["server2"].as<String>();
    m_settings.dateTime.gmtOffset = jsonObj["date_time"]["gmt_offset"].as<long>();
    m_settings.dateTime.daylightOffset = jsonObj["date_time"]["daylight_offset"].as<int>();

    m_settings.logging.level = jsonObj["logging"]["level"].as<uint8_t>();
    m_settings.logging.refreshPeriod = jsonObj["logging"]["refresh_period"].as<uint16_t>();

    return true;
}

String Settings::createJson() {
    StaticJsonDocument<1024> doc;

    JsonObject deviceObj = doc.createNestedObject("device");
    deviceObj["device_id"] = m_settings.app.deviceID;
    deviceObj["geo_location_lat"] = m_settings.app.geoLocation.lat;
    deviceObj["geo_location_lng"] = m_settings.app.geoLocation.lng;

    JsonObject mqttObj = doc.createNestedObject("mqtt");
    mqttObj["server"] = m_settings.mqtt.connection.server;
    mqttObj["port"] = m_settings.mqtt.connection.port;
    mqttObj["username"] = m_settings.mqtt.connection.username;
    mqttObj["password"] = m_settings.mqtt.connection.password;
    mqttObj["crt_path"] = m_settings.mqtt.connection.caCertPath;

    JsonArray wifiArr = doc.createNestedArray("wifi");
    for (size_t i = 0; i < m_settings.wifiAPs.size(); i++) {
        JsonObject wifiAP = wifiArr.createNestedObject();
        wifiAP["ssid"] = m_settings.wifiAPs[i].ssid;
        wifiAP["password"] = m_settings.wifiAPs[i].password;
    }

    JsonObject dateTimeObj = doc.createNestedObject("date_time");
    dateTimeObj["server1"] = m_settings.dateTime.server1;
    dateTimeObj["server2"] = m_settings.dateTime.server2;
    dateTimeObj["gmt_offset"] = m_settings.dateTime.gmtOffset;
    dateTimeObj["daylight_offset"] = m_settings.dateTime.daylightOffset;

    JsonObject debugging = doc.createNestedObject("logging");
    debugging["level"] = m_settings.logging.level;
    debugging["refresh_period"] = m_settings.logging.refreshPeriod;

    String json;
    serializeJsonPretty(doc, json);

    return json;
}

void Settings::defaultSettings() {
    m_settings.app.deviceID = m_app->deviceID();
    m_settings.app.geoLocation.lat = "";
    m_settings.app.geoLocation.lng = "";

    m_settings.mqtt.connection.server = "";
    m_settings.mqtt.connection.port = 0;
    m_settings.mqtt.connection.username = "";
    m_settings.mqtt.connection.password = "";
    m_settings.mqtt.connection.caCertPath = "/settings/mqtt_ca_root.crt";

    m_settings.wifiAPs.clear();

    m_settings.dateTime.server1 = "pool.ntp.org";
    m_settings.dateTime.server2 = "time.nist.gov";
    m_settings.dateTime.gmtOffset = -3;
    m_settings.dateTime.daylightOffset = 0;

    m_settings.logging.level = LOG_LEVEL_DEBUG;
    m_settings.logging.refreshPeriod = 0;
}
