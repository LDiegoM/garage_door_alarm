#include <internal/net/wifi_connection.h>

//////////////////// Constructor
WiFiConnection::WiFiConnection(Settings *settings) {
    m_settings = settings;

#ifdef ESP8266
    m_wifiMulti = new ESP8266WiFiMulti();
#else
    m_wifiMulti = new WiFiMulti();
#endif
}

//////////////////// Public methods implementation
bool WiFiConnection::begin() {
    if (!m_settings->isSettingsOK())
        return false;

    for (size_t i = 0; i < m_settings->getSettings().wifiAPs.size(); i++) {
        m_wifiMulti->addAP(m_settings->getSettings().wifiAPs[i].ssid.c_str(),
                           m_settings->getSettings().wifiAPs[i].password.c_str());
    }

    if (!connect()) {
        // If no wifi ap could connect, then begin default AP to enable configuration.
        return beginAP();
    }
    return true;
}

bool WiFiConnection::connect() {
    if (m_wifiMulti->run() != WL_CONNECTED)
        return false;

    return true;
}

bool WiFiConnection::beginAP() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(SSID_AP))
        return false;

    return true;
}

bool WiFiConnection::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiConnection::isModeAP() {
    return WiFi.getMode() == WIFI_AP;
}

String WiFiConnection::getIP() {
    if (WiFi.getMode() == WIFI_AP)
        return WiFi.softAPIP().toString();

    if (!isConnected())
        return "";

    return WiFi.localIP().toString();
}

String WiFiConnection::getSSID() {
    if (WiFi.getMode() == WIFI_AP)
        return SSID_AP;
    
    if (WiFi.status() == WL_CONNECTED)
        return WiFi.SSID();
    
    return "";
}
