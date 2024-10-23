#ifndef wifi_connection_h
#define wifi_connection_h

#include <vector>

#ifdef ESP8266
#include <ESP8266WiFiMulti.h>
#elif defined(ESP32)
#include <WiFiMulti.h>
#else
#error "Unsupported platform"
#endif

#include <internal/core/logging.h>

struct wifiAP_t {
    String ssid, password;
};

class WiFiConnection {
    private:
        const char *SSID_AP = "esp_application";

#ifdef ESP8266
        ESP8266WiFiMulti *m_wifiMulti;
#elif defined(ESP32)
        WiFiMulti *m_wifiMulti;
#endif
        std::vector<wifiAP_t> m_wifiAPs;
        String m_apSSID = "";

        String getStatusFromInt(uint8_t status);

    public:
        WiFiConnection();
        WiFiConnection(String apSSID);
        WiFiConnection(std::vector<wifiAP_t> wifiAPs);
        WiFiConnection(std::vector<wifiAP_t> wifiAPs, String apSSID);

        bool begin();
        bool connect(bool verbose);
        bool beginAP();
        bool beginAP(String apSSID);
        String getDeviceAPSSID();
        bool isConnected();
        bool isModeAP();
        String getIP();
        String getSSID();
        String getStatus();
};

#endif
