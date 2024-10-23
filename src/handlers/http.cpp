#include <handlers/http.h>

HttpHandlers *httpHandlers = nullptr;

//////////////////// Constructor
HttpHandlers::HttpHandlers(Application *app, Settings *settings, CommonDoorStatus *doorStatus) {
    m_app = app;
    m_settings = settings;
    m_doorStatus = doorStatus;
}

//////////////////// Public methods implementation
bool HttpHandlers::begin() {
    if (!m_settings->isSettingsOK())
        return false;

#ifdef ESP8266
    m_server = new ESP8266WebServer(METEO_HTTP_PORT);
#elif defined(ESP32)
    m_server = new WebServer(METEO_HTTP_PORT);
#endif

    defineRoutes();

    m_server->begin();

    return true;
}

void HttpHandlers::loop() {
    m_server->handleClient();
}

/////////// HTTP Handlers
void HttpHandlers::handleDownloadLogs() {
    if (!m_app->storage()->exists(LOGGING_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    File file = m_app->storage()->open(LOGGING_FILE);
    if (!file) {
        m_server->send(500, "text/plain", "fail to open logs file");
        return;
    }
    size_t fileSize = file.size();

    String dataType = "application/octet-stream";

    m_server->sendHeader("Content-Disposition", "inline; filename=logs.txt");

    size_t streamSize = m_server->streamFile(file, dataType);
    if (streamSize != fileSize)
        lg->warn("handleDownloadLogs() - sent different data length than expected", __FILE__, __LINE__,
            lg->newTags()
                ->add("file_size", String(fileSize))
                ->add("sent", String(streamSize))
        );
    
    file.close();
}
bool HttpHandlers::handleDeleteLogs() {
    if (!m_app->storage()->exists(LOGGING_FILE)) {
        m_server->send(204);
        return false;
    }

    bool flgOK = m_app->storage()->remove(LOGGING_FILE);
    if (flgOK) {
        lg->warn("logging file was removed by http request", __FILE__, __LINE__);
        m_server->send(204);
    } else {
        lg->error("http handler couldn't remove logging file", __FILE__, __LINE__);
        m_server->send(500, "text/plain", "could not delete logging file");
    }

    return flgOK;
}
void HttpHandlers::handleRestart() {
    lg->warn("device was restarted by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
    ESP.restart();
}
void HttpHandlers::handleGetStatus() {
    m_server->send(200, "text/plain", m_doorStatus->getCurrentDoorStatus());
}
void HttpHandlers::handleGetSettings() {
    if (!m_app->storage()->exists(SETTINGS_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    m_server->send(200, "application/json", m_app->storage()->readAll(SETTINGS_FILE));
}
void HttpHandlers::handleDelSettings() {
    if (!m_app->storage()->exists(SETTINGS_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    if (!m_app->storage()->remove(SETTINGS_FILE)) {
        lg->error("http handlers couldn't remove settings", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->warn("all settings were removed and set to defaults by http request", __FILE__, __LINE__);

    handleRestart();
}

void HttpHandlers::handleGetBootstrapCSS() {
    File file = m_app->storage()->open("/wwwroot/bootstrap.min.css.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/css");
    file.close();
}
void HttpHandlers::handleGetBootstrapJS() {
    File file = m_app->storage()->open("/wwwroot/bootstrap.bundle.min.js.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/js");
    file.close();
}
void HttpHandlers::handleGetNotFound() {
    String html = getNotFoundHTML();
    m_server->send(404, "text/html", html);
}

void HttpHandlers::handleGetStatusHTML() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("status"));
    m_server->sendContent(getStatusHTML());
    m_server->sendContent(getFooterHTML());
}

void HttpHandlers::handleGetSettingsDevice() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsDeviceHTML());
    m_server->sendContent(getFooterHTML());
}
void HttpHandlers::handleUpdSettingsDevice() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    request_device_t deviceSettings = parseDeviceBody(body);
    if (deviceSettings.deviceID.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    m_app->setDeviceID(deviceSettings.deviceID);
    geoLocation_t appGeoLocation;
    appGeoLocation.lat = deviceSettings.geoLocationLat;
    appGeoLocation.lng = deviceSettings.geoLocationLng;
    m_app->setGeoLocation(appGeoLocation);
    m_settings->setDeviceValue(deviceSettings.deviceID, appGeoLocation.lat, appGeoLocation.lng);

    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to update device", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("device settings were updated by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetSettingsWiFi() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsWiFiHTML());
    m_server->sendContent(getFooterHTML());
}
void HttpHandlers::handleAddSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    request_wifiAP_t newWiFiAP = parseWiFiBody(body);
    if (newWiFiAP.ssid.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    if (m_settings->ssidExists(newWiFiAP.ssid)) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_EXISTS);
        return;
    }

    m_settings->addWifiAP(newWiFiAP.ssid.c_str(), newWiFiAP.password.c_str());
    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to add wifi ap", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("new wifi ap was added by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleUpdSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    std::vector<request_wifiAP_t> aps = parseMultiWiFiBody(body);
    if (aps.size() < 1) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    for (size_t i = 0; i < aps.size(); i++) {
        if (!m_settings->updWifiAP(aps[i].ssid.c_str(), aps[i].password.c_str())) {
            m_server->send(404, "text/plain", ERR_WIFI_AP_NOT_FOUND);
            return;
        }
    }

    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to update wifi aps", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("wifi aps were updated by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleDelSettingsWiFi() {
    String ssid = m_server->arg("ap");
    if (ssid.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    if (!m_settings->ssidExists(ssid)) {
        m_server->send(404, "text/plain", ERR_WIFI_AP_NOT_FOUND);
        return;
    }

    m_settings->delWifiAP(ssid.c_str());
    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetSettingsMQTT() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsMQTTHTML());
    m_server->sendContent(getFooterHTML());
}
void HttpHandlers::handleUpdSettingsMQTT() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    request_mqtt_t mqttValues = parseMQTTBody(body);
    if (mqttValues.server.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    if (mqttValues.password.equals("****"))
        m_settings->setMQTTValues(mqttValues.server, mqttValues.username, mqttValues.port);
    else
        m_settings->setMQTTValues(mqttValues.server, mqttValues.username, mqttValues.password, mqttValues.port);

    if (!mqttValues.certData.equals("")) {
        if (!m_settings->setMQTTCertificate(mqttValues.certData)) {
            lg->error("http handler couldn't save mqtt certificate", __FILE__, __LINE__);
            m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
            return;        
        }
    }

    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to update mqqt", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("mqtt settings were updated by htt request", __FILE__, __LINE__);

    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleGetSettingsMQTTCert() {
    m_server->send(200, "text/plain", m_settings->getSettings().mqtt.connection.ca_cert);
}

void HttpHandlers::handleGetSettingsDate() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsDateHTML());
    m_server->sendContent(getFooterHTML());
}
void HttpHandlers::handleUpdSettingsDate() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    request_dateTime_t dateSettings = parseDateBody(body);
    if (dateSettings.server1.equals("") || dateSettings.server2.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    m_settings->setDateValues(dateSettings.server1, dateSettings.server2, dateSettings.gmtOffset, dateSettings.daylightOffset);

    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to update date", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("date settings were updated by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetSettingsLogging() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsLoggingHTML());
    m_server->sendContent(getFooterHTML());
}
void HttpHandlers::handleUpdSettingsLogging() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_EMPTY_PARAMETERS);
        return;
    }

    request_logging_t loggingSettings = parseLoggingBody(body);

    m_settings->setLoggingValues(loggingSettings.level, loggingSettings.refreshPeriod);

    if (!m_settings->saveSettings()) {
        lg->error("http handler couldn't save settings to update logging", __FILE__, __LINE__);
        m_server->send(500, "text/plain", ERR_SETTINGS_SAVE_GENERIC);
        return;        
    }

    lg->info("logging settings were updated by http request", __FILE__, __LINE__);
    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetAdmin() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("admin"));
    m_server->sendContent(getAdminHTML());
    m_server->sendContent(getFooterHTML());
}

//////////////////// Private methods implementation
void HttpHandlers::defineRoutes() {
    m_server->on("/", HTTP_GET, [](){httpHandlers->handleGetStatusHTML();});
    m_server->on("/admin", HTTP_GET, [](){httpHandlers->handleGetAdmin();});

    m_server->on("/logs", HTTP_GET, [](){httpHandlers->handleDownloadLogs();});
    m_server->on("/logs", HTTP_DELETE, [](){httpHandlers->handleDeleteLogs();});

    m_server->on("/restart", HTTP_POST, [](){httpHandlers->handleRestart();});
    m_server->on("/status", HTTP_GET, [](){httpHandlers->handleGetStatus();});

    m_server->on("/settings", HTTP_GET, [](){httpHandlers->handleGetSettings();});
    m_server->on("/settings", HTTP_DELETE, [](){httpHandlers->handleDelSettings();});

    m_server->on("/bootstrap.min.css", HTTP_GET, [](){httpHandlers->handleGetBootstrapCSS();});
    m_server->on("/bootstrap.bundle.min.js", HTTP_GET, [](){httpHandlers->handleGetBootstrapJS();});

    m_server->on("/settings/device", HTTP_GET, [](){httpHandlers->handleGetSettingsDevice();});
    m_server->on("/settings/device", HTTP_PUT, [](){httpHandlers->handleUpdSettingsDevice();});

    m_server->on("/settings/wifi", HTTP_GET, [](){httpHandlers->handleGetSettingsWiFi();});
    m_server->on("/settings/wifi", HTTP_POST, [](){httpHandlers->handleAddSettingsWiFi();});
    m_server->on("/settings/wifi", HTTP_PUT, [](){httpHandlers->handleUpdSettingsWiFi();});
    m_server->on("/settings/wifi", HTTP_DELETE, [](){httpHandlers->handleDelSettingsWiFi();});

    m_server->on("/settings/mqtt", HTTP_GET, [](){httpHandlers->handleGetSettingsMQTT();});
    m_server->on("/settings/mqtt", HTTP_PUT, [](){httpHandlers->handleUpdSettingsMQTT();});
    m_server->on("/settings/mqtt/cert", HTTP_GET, [](){httpHandlers->handleGetSettingsMQTTCert();});

    m_server->on("/settings/date", HTTP_GET, [](){httpHandlers->handleGetSettingsDate();});
    m_server->on("/settings/date", HTTP_PUT, [](){httpHandlers->handleUpdSettingsDate();});

    m_server->on("/settings/logging", HTTP_GET, [](){httpHandlers->handleGetSettingsLogging();});
    m_server->on("/settings/logging", HTTP_PUT, [](){httpHandlers->handleUpdSettingsLogging();});

    m_server->onNotFound([](){httpHandlers->handleGetNotFound();});
}

request_device_t HttpHandlers::parseDeviceBody(String body) {
    request_device_t deviceValues;

    StaticJsonDocument<256> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse device settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return deviceValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    deviceValues.deviceID = jsonObj["device_id"].as<String>();
    deviceValues.geoLocationLat = jsonObj["geo_location_lat"].as<String>();
    deviceValues.geoLocationLng = jsonObj["geo_location_lng"].as<String>();

    return deviceValues;
}

request_wifiAP_t HttpHandlers::parseWiFiBody(String body) {
    request_wifiAP_t newWiFiAP;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse wifi settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return newWiFiAP;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    newWiFiAP.ssid = jsonObj["ap"].as<String>();
    newWiFiAP.password = jsonObj["pw"].as<String>();

    return newWiFiAP;
}

std::vector<request_wifiAP_t> HttpHandlers::parseMultiWiFiBody(String body) {
    std::vector<request_wifiAP_t> aps;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse multi-wifi settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return aps;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    for (size_t i = 0; i < jsonObj["aps"].size(); i++) {
        request_wifiAP_t ap;
        ap.ssid = jsonObj["aps"][i]["ap"].as<String>();
        ap.password = jsonObj["aps"][i]["pw"].as<String>();

        aps.push_back(ap);
    }

    return aps;
}

request_mqtt_t HttpHandlers::parseMQTTBody(String body) {
    request_mqtt_t mqttValues;

    StaticJsonDocument<4096> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse MQTT settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return mqttValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    mqttValues.server = jsonObj["server"].as<String>();
    mqttValues.username = jsonObj["user"].as<String>();
    mqttValues.password = jsonObj["pw"].as<String>();
    mqttValues.port = jsonObj["port"].as<uint16_t>();
    mqttValues.sendPeriod = jsonObj["send_period"].as<uint16_t>();

    String cert = "";
    for (size_t i = 0; i < jsonObj["cert"].size(); i++) {
        cert += jsonObj["cert"][i].as<String>() + "\n";
    }
    mqttValues.certData = cert;

    return mqttValues;
}

request_dateTime_t HttpHandlers::parseDateBody(String body) {
    request_dateTime_t dateValues;

    StaticJsonDocument<256> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse date settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return dateValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    dateValues.server1 = jsonObj["server1"].as<String>();
    dateValues.server2 = jsonObj["server2"].as<String>();
    dateValues.gmtOffset = jsonObj["gmt_offset"].as<long>();
    dateValues.daylightOffset = jsonObj["daylight_offset"].as<int>();

    return dateValues;
}

request_logging_t HttpHandlers::parseLoggingBody(String body) {
    request_logging_t loggingValues;

    StaticJsonDocument<256> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        lg->error("fail to parse logging settings body", __FILE__, __LINE__,
            lg->newTags()
                ->add("method", "deserializeJson()")
                ->add("error", String(error.c_str()))
        );
        return loggingValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    loggingValues.level = jsonObj["level"].as<uint8_t>();
    loggingValues.refreshPeriod = jsonObj["refresh_period"].as<uint16_t>();

    return loggingValues;
}
