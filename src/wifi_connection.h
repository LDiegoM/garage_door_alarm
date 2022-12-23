#ifndef wifi_connection_h
#define wifi_connection_h

#ifdef ESP8266
#include <ESP8266WiFiMulti.h>
#elif defined(ESP32)
#include <WiFiMulti.h>
#else
#error "Unsupported platform"
#endif

#include <settings.h>

class WiFiConnection {
    private:
        const char *SSID_AP = "garage_door_alarm";

#ifdef ESP8266
        ESP8266WiFiMulti *m_wifiMulti;
#elif defined(ESP32)
        WiFiMulti *m_wifiMulti;
#endif
        Settings *m_settings;

    public:
        WiFiConnection(Settings *settings);

        bool begin();
        bool connect();
        bool beginAP();
        bool isConnected();
        bool isModeAP();
        String getIP();
        String getSSID();
};

#endif
