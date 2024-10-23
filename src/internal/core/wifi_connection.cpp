#include <internal/core/wifi_connection.h>

//////////////////// Constructor
WiFiConnection::WiFiConnection() {
    m_apSSID = String(SSID_AP);
}
WiFiConnection::WiFiConnection(String apSSID) {
    m_apSSID = apSSID;
}
WiFiConnection::WiFiConnection(std::vector<wifiAP_t> wifiAPs) {
    m_apSSID = String(SSID_AP);
    m_wifiAPs = wifiAPs;

#ifdef ESP8266
    m_wifiMulti = new ESP8266WiFiMulti();
#else
    m_wifiMulti = new WiFiMulti();
#endif
}
WiFiConnection::WiFiConnection(std::vector<wifiAP_t> wifiAPs, String apSSID) {
    m_apSSID = apSSID;
    m_wifiAPs = wifiAPs;

#ifdef ESP8266
    m_wifiMulti = new ESP8266WiFiMulti();
#else
    m_wifiMulti = new WiFiMulti();
#endif
}

//////////////////// Public methods implementation
bool WiFiConnection::begin() {
    if (m_wifiMulti == nullptr)
        return beginAP();

    for (size_t i = 0; i < m_wifiAPs.size(); i++) {
        m_wifiMulti->addAP(m_wifiAPs[i].ssid.c_str(),
                           m_wifiAPs[i].password.c_str());
    }

    if (!connect(true)) {
        // If no wifi ap could connect, then begin default AP to enable configuration.
        return beginAP();
    }
    return true;
}

bool WiFiConnection::connect(bool verbose) {
    if (verbose) {
        lg->debug("WiFi connecting", __FILE__, __LINE__);
    }

    uint8_t result = m_wifiMulti->run();
    if (result != WL_CONNECTED) {
        if (verbose)
            lg->warn("WiFi connection failed", __FILE__, __LINE__, lg->newTags()->add("status", getStatusFromInt(result)));
        return false;
    }

    if (verbose) {
        lg->info("WiFi connected", __FILE__, __LINE__,
            lg->newTags()
                ->add("status", getStatusFromInt(result))
                ->add("mode", isModeAP() ? "AP" : "Client")
                ->add("ssid", getSSID())
                ->add("ip", getIP())
        );
    }
    return true;
}

bool WiFiConnection::beginAP() {
    return beginAP(m_apSSID);
}

bool WiFiConnection::beginAP(String apSSID) {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(apSSID.c_str())) {
        lg->error("AP init Error", __FILE__, __LINE__);
        return false;
    }
    lg->info("WiFi AP initiated", __FILE__, __LINE__,
        lg->newTags()
            ->add("status", getStatus())
            ->add("mode", isModeAP() ? "AP" : "Client")
            ->add("ssid", getSSID())
            ->add("ip", getIP())
    );
    return true;
}

String WiFiConnection::getDeviceAPSSID() {
    return m_apSSID;
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
        return m_apSSID;
    
    if (WiFi.status() == WL_CONNECTED)
        return WiFi.SSID();
    
    return "";
}

String WiFiConnection::getStatus() {
    return getStatusFromInt(WiFi.status());
}

//////////////////// Private methods implementation
String WiFiConnection::getStatusFromInt(uint8_t status) {
    switch (status) {
        case 0:
            return "WL_IDLE_STATUS";
        case 1:
            return "WL_NO_SSID_AVAIL";
        case 2:
            return "WL_SCAN_COMPLETED";
        case 3:
            return "WL_CONNECTED";
        case 4:
            return "WL_CONNECT_FAILED";
        case 5:
            return "WL_CONNECTION_LOST";
        case 6:
            return "WL_DISCONNECTED";
        case 255:
            return "WL_NO_SHIELD";
    }
    return "INVALID_STATUS";
}
