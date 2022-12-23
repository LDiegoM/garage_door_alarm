#include <http_handlers.h>

HttpHandlers *httpHandlers = nullptr;

//////////////////// HTTP Handlers
void downloadLogs(void) {
    httpHandlers->handleDownloadLogs();
}
void deleteLogs(void) {
    if (httpHandlers->handleDeleteLogs())
        dataLogger->logData();
}
void restart() {
    httpHandlers->handleRestart();
}
void getStatus() {
    httpHandlers->handleGetStatus();
}
void getSettings() {
    httpHandlers->handleGetSettings();
}
void delSettings() {
    httpHandlers->handleDelSettings();
}

void getBootstrapCSS() {
    httpHandlers->handleGetBootstrapCSS();
}
void getBootstrapJS() {
    httpHandlers->handleGetBootstrapJS();
}
void getNotFound() {
    httpHandlers->handleGetNotFound();
}

void getStatusHTML() {
    httpHandlers->handleGetStatusHTML();
}

void getSettingsWiFi() {
    httpHandlers->handleGetSettingsWiFi();
}
void addSettingsWiFi() {
    httpHandlers->handleAddSettingsWiFi();
}
void updSettingsWiFi() {
    httpHandlers->handleUpdSettingsWiFi();
}
void delSettingsWiFi() {
    httpHandlers->handleDelSettingsWiFi();
}

void getSettingsMQTT() {
    httpHandlers->handleGetSettingsMQTT();
}
void updSettingsMQTT() {
    httpHandlers->handleUpdSettingsMQTT();
}
void getSettingsMQTTCert() {
    httpHandlers->handleGetSettingsMQTTCert();
}

void getSettingsDate() {
    httpHandlers->handleGetSettingsDate();
}
void updSettingsDate() {
    httpHandlers->handleUpdSettingsDate();
}

void getAdmin() {
    httpHandlers->handleGetAdmin();
}

//////////////////// Constructor
HttpHandlers::HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings,
                           DataLogger *dataLogger, DoorStatus *doorStatus, MqttHandlers *mqtt) {
    m_wifi = wifi;
    m_storage = storage;
    m_settings = settings;
    m_dataLogger = dataLogger;
    m_doorStatus = doorStatus;
    m_mqtt = mqtt;
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
    if (!m_storage->exists(m_settings->getSettings().logger.outputPath.c_str())) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    File file = m_storage->open(m_settings->getSettings().logger.outputPath.c_str());
    if (!file) {
        m_server->send(500, "text/plain", "fail to open log file");
        return;
    }

    String dataType = "application/octet-stream";

    m_server->sendHeader("Content-Disposition", "inline; filename=garage_door_logs.txt");

    if (m_server->streamFile(file, dataType) != file.size())
        Serial.println("Sent different data length than expected");
    
    file.close();
}
bool HttpHandlers::handleDeleteLogs() {
    if (!m_storage->exists(m_settings->getSettings().logger.outputPath.c_str())) {
        m_server->send(404, "text/plain", "not found");
        return false;
    }

    bool flgOK = m_storage->remove(m_settings->getSettings().logger.outputPath.c_str());
    if (flgOK)
        m_server->send(204);
    else
        m_server->send(500, "text/plain", "could not delete file");
    
    return flgOK;
}
void HttpHandlers::handleRestart() {
    m_server->send(200, "text/plain", MSG_OK);
    ESP.restart();
}
void HttpHandlers::handleGetStatus() {
    m_server->send(200, "text/plain", m_doorStatus->getCurrentDoorStatus());
}
void HttpHandlers::handleGetSettings() {
    if (!m_storage->exists(SETTINGS_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    m_server->send(200, "application/json", m_storage->readAll(SETTINGS_FILE));
}
void HttpHandlers::handleDelSettings() {
    if (!m_storage->exists(SETTINGS_FILE)) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    if (!m_storage->remove(SETTINGS_FILE)) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    handleRestart();
}

void HttpHandlers::handleGetBootstrapCSS() {
    File file = m_storage->open("/wwwroot/bootstrap.min.css.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/css");
    file.close();
}
void HttpHandlers::handleGetBootstrapJS() {
    File file = m_storage->open("/wwwroot/bootstrap.bundle.min.js.gz");
    if (!file) {
        m_server->send(404);
        return;
    }
    m_server->streamFile(file, "text/js");
    file.close();
}
void HttpHandlers::handleGetNotFound() {
    String html = m_storage->readAll("/wwwroot/error.html");
    html.replace("{error_description}", "Resource not found");
    m_server->send(404, "text/html", html);
}

void HttpHandlers::handleGetStatusHTML() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("status"));
    m_server->sendContent(getStatusHTMLPage());
    m_server->sendContent(getFooterHTML("status", ""));
}

void HttpHandlers::handleGetSettingsWiFi() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsWiFiHTMLPage());
    m_server->sendContent(getFooterHTML("settings", "wifi"));
}
void HttpHandlers::handleAddSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    wifiAP_t newWiFiAP = parseWiFiBody(body);
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
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleUpdSettingsWiFi() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_WIFI_AP_IS_EMPTY);
        return;
    }

    std::vector<wifiAP_t> aps = parseMultiWiFiBody(body);
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
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

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
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetSettingsMQTT() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsMQTTHTMLPage());
    m_server->sendContent(getFooterHTML("settings", "mqtt"));
}
void HttpHandlers::handleUpdSettingsMQTT() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_MQTT_IS_EMPTY);
        return;
    }

    settings_mqtt_t mqttValues = parseMQTTBody(body);
    if (mqttValues.server.equals("")) {
        m_server->send(400, "text/plain", ERR_MQTT_IS_EMPTY);
        return;
    }

    if (mqttValues.password.equals("****"))
        m_settings->setMQTTValues(mqttValues.server, mqttValues.username, mqttValues.port);
    else
        m_settings->setMQTTValues(mqttValues.server, mqttValues.username, mqttValues.password, mqttValues.port);

    if (!mqttValues.certData.equals("")) {
        if (!m_settings->setMQTTCertificate(mqttValues.certData)) {
            m_server->send(500, "text/plain", ERR_GENERIC);
            return;        
        }
    }

    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}
void HttpHandlers::handleGetSettingsMQTTCert() {
    m_server->send(200, "text/plain", m_settings->getSettings().mqtt.ca_cert);
}

void HttpHandlers::handleGetSettingsDate() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("settings"));
    m_server->sendContent(getSettingsDateHTMLPage());
    m_server->sendContent(getFooterHTML("settings", "date"));
}
void HttpHandlers::handleUpdSettingsDate() {
    String body = m_server->arg("plain");
    if (body.equals("")) {
        m_server->send(400, "text/plain", ERR_DATE_IS_EMPTY);
        return;
    }

    settings_date_t dateSettings = parseDateBody(body);
    if (dateSettings.server.equals("")) {
        m_server->send(400, "text/plain", ERR_DATE_IS_EMPTY);
        return;
    }

    m_settings->setDateValues(dateSettings.server, dateSettings.gmtOffset, dateSettings.daylightOffset);

    if (!m_settings->saveSettings()) {
        m_server->send(500, "text/plain", ERR_GENERIC);
        return;        
    }

    m_server->send(200, "text/plain", MSG_OK);
}

void HttpHandlers::handleGetAdmin() {
    m_server->sendHeader("Content-Type", "text/html");
    m_server->sendContent(getHeaderHTML("admin"));
    m_server->sendContent(getAdminHTMLPage());
    m_server->sendContent(getFooterHTML("admin", "admin"));
}

//////////////////// Private methods implementation
void HttpHandlers::defineRoutes() {
    m_server->on("/", HTTP_GET, getStatusHTML);

    m_server->on("/logs", HTTP_GET, downloadLogs);
    m_server->on("/logs", HTTP_DELETE, deleteLogs);
    m_server->on("/restart", HTTP_POST, restart);
    m_server->on("/measures", HTTP_GET, getStatus);
    m_server->on("/settings", HTTP_GET, getSettings);
    m_server->on("/settings", HTTP_DELETE, delSettings);

    m_server->on("/bootstrap.min.css", HTTP_GET, getBootstrapCSS);
    m_server->on("/bootstrap.bundle.min.js", HTTP_GET, getBootstrapJS);

    m_server->on("/settings/wifi", HTTP_GET, getSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_POST, addSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_PUT, updSettingsWiFi);
    m_server->on("/settings/wifi", HTTP_DELETE, delSettingsWiFi);

    m_server->on("/settings/mqtt", HTTP_GET, getSettingsMQTT);
    m_server->on("/settings/mqtt", HTTP_PUT, updSettingsMQTT);
    m_server->on("/settings/mqtt/cert", HTTP_GET, getSettingsMQTTCert);

    m_server->on("/settings/date", HTTP_GET, getSettingsDate);
    m_server->on("/settings/date", HTTP_PUT, updSettingsDate);

    m_server->on("/admin", HTTP_GET, getAdmin);

    m_server->onNotFound(getNotFound);
}

String HttpHandlers::getHeaderHTML(String section) {
    String header = m_storage->readAll("/wwwroot/header.html");

    header.replace("{active_status}", (section.equals("status") ? " active" : ""));
    header.replace("{active_settings}", (section.equals("settings") ? " active" : ""));
    header.replace("{active_admin}", (section.equals("admin") ? " active" : ""));

    if (!m_wifi->isModeAP() && m_mqtt->isConnected()) {
        header.replace("/bootstrap.min.css", BOOTSTRAP_CSS);
        header.replace("/bootstrap.bundle.min.js", BOOTSTRAP_JS);
    }

    return header;
}

String HttpHandlers::getFooterHTML(String page, String section) {
    String footer = m_storage->readAll("/wwwroot/footer.html");
    String js = "";
    js += "<script>";
    js += m_storage->readAll("/wwwroot/utils.js");
    js += "\n";
    if (!page.equals("") && !section.equals(""))
        js += m_storage->readAll((String("/wwwroot/") + page + "/" + section + ".js").c_str());
    js += "</script>";
    footer.replace("<!--{utils.js}-->", js);

    return footer;
}

String HttpHandlers::getStatusHTMLPage() {
    String html = m_storage->readAll("/wwwroot/status/status.html");

    html.replace("{door_status}", m_doorStatus->getCurrentDoorStatus());

    if (m_wifi->isModeAP()) {
        html.replace("{wifi_connected}", DISCONNECTED);
        html.replace("{ssid}", "AP: " + m_wifi->getSSID());
        html.replace("{ip}", "IP: " + m_wifi->getIP());
    } else if (m_wifi->isConnected()) {
        html.replace("{wifi_connected}", CONNECTED);
        html.replace("{ssid}", "SSID: " + m_wifi->getSSID());
        html.replace("{ip}", "IP: " + m_wifi->getIP());
    } else {
        html.replace("{wifi_connected}", DISCONNECTED);
        html.replace("{ssid}", "");
        html.replace("{ip}", "");
    }

    html.replace("{freeMem}", String((float) ESP.getFreeHeap() / 1024) + " kb");

    if (m_mqtt->isConnected())
        html.replace("{mqtt_connected}", CONNECTED);
    else
        html.replace("{mqtt_connected}", DISCONNECTED);

    return html;
}

String HttpHandlers::getSettingsWiFiHTMLPage() {
    String htmlUpdate = "";
    for (size_t i = 0; i < m_settings->getSettings().wifiAPs.size(); i++) {
        String htmlAP = m_storage->readAll("/wwwroot/settings/wifi_update_ap.html");
        htmlAP.replace("{ap_name}", m_settings->getSettings().wifiAPs[i].ssid);
        htmlAP += "\n";
        htmlUpdate += htmlAP;
    }

    String html = m_storage->readAll("/wwwroot/settings/wifi.html");
    html.replace("<!--{wifi_update_ap.html}-->", htmlUpdate);
    return html;
}

String HttpHandlers::getSettingsMQTTHTMLPage() {
    String html = m_storage->readAll("/wwwroot/settings/mqtt.html");

    settings_t settings = m_settings->getSettings();
    html.replace("{server}", settings.mqtt.server);
    html.replace("{user}", settings.mqtt.username);
    html.replace("{port}", String(settings.mqtt.port));
    html.replace("{certificate}", "");

    return html;
}

String HttpHandlers::getSettingsDateHTMLPage() {
    String html = m_storage->readAll("/wwwroot/settings/date.html");

    settings_t settings = m_settings->getSettings();
    html.replace("{server}", String(settings.dateTime.server));
    html.replace("{gmtOffset}", String(settings.dateTime.gmtOffset));
    html.replace("{daylightOffset}", String(settings.dateTime.daylightOffset));

    return html;
}

String HttpHandlers::getAdminHTMLPage() {
    String html = m_storage->readAll("/wwwroot/admin/admin.html");

    html.replace("{free_storage}", m_storage->getFree());
    html.replace("{logs_size}", m_dataLogger->logSize());
    html.replace("{last_date}", m_dataLogger->getLastLogTime());

    return html;
}

wifiAP_t HttpHandlers::parseWiFiBody(String body) {
    wifiAP_t newWiFiAP;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return newWiFiAP;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    newWiFiAP.ssid = jsonObj["ap"].as<String>();
    newWiFiAP.password = jsonObj["pw"].as<String>();

    return newWiFiAP;
}

std::vector<wifiAP_t> HttpHandlers::parseMultiWiFiBody(String body) {
    std::vector<wifiAP_t> aps;

    StaticJsonDocument<250> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return aps;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    for (size_t i = 0; i < jsonObj["aps"].size(); i++) {
        wifiAP_t ap;
        ap.ssid = jsonObj["aps"][i]["ap"].as<String>();
        ap.password = jsonObj["aps"][i]["pw"].as<String>();

        aps.push_back(ap);
    }

    return aps;
}

settings_mqtt_t HttpHandlers::parseMQTTBody(String body) {
    settings_mqtt_t mqttValues;

    StaticJsonDocument<4096> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return mqttValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    mqttValues.server = jsonObj["server"].as<String>();
    mqttValues.username = jsonObj["user"].as<String>();
    mqttValues.password = jsonObj["pw"].as<String>();
    mqttValues.port = jsonObj["port"].as<uint16_t>();

    String cert = "";
    for (size_t i = 0; i < jsonObj["cert"].size(); i++) {
        cert += jsonObj["cert"][i].as<String>() + "\n";
    }
    mqttValues.certData = cert;

    return mqttValues;
}

settings_date_t HttpHandlers::parseDateBody(String body) {
    settings_date_t dateValues;

    StaticJsonDocument<256> configs;
    DeserializationError error = deserializeJson(configs, body);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return dateValues;
    }
    JsonObject jsonObj = configs.as<JsonObject>();

    dateValues.server = jsonObj["server"].as<String>();
    dateValues.gmtOffset = jsonObj["gmt_offset"].as<long>();
    dateValues.daylightOffset = jsonObj["daylight_offset"].as<int>();

    return dateValues;
}
