#ifndef settings_h
#define settings_h

#include <vector>
#include <ArduinoJson.h>
#include <internal/low_level/storage.h>

struct wifiAP_t {
    String ssid, password;
};

struct settings_t {
    struct mqtt {
        String server, username, password, caCertPath;
        uint16_t port;
        char *ca_cert;
    } mqtt;
    std::vector<wifiAP_t> wifiAPs;
    struct logger {
        String outputPath;
    } logger;
    struct dateTime {
        String server;
        long gmtOffset;
        int daylightOffset;
    } dateTime;
};

extern const char* SETTINGS_FILE;

class Settings {
    private:
        Storage *m_storage;
        settings_t m_settings;
        bool m_settingsOK;

        bool readSettings();
        String createJson();
        void defaultSettings();

    public:
        Settings(Storage *storage);

        bool begin();
        bool isSettingsOK();
        settings_t getSettings();
        bool saveSettings();

        void addWifiAP(const char* ssid, const char* password);
        bool updWifiAP(const char* ssid, const char* password);
        bool delWifiAP(const char* ssid);
        bool ssidExists(String ssid);

        void setMQTTValues(String server, String username, String password, uint16_t port);
        void setMQTTValues(String server, String username, uint16_t port);
        bool setMQTTCertificate(String certData);

        void setDateValues(String server, long gmtOffset, int daylightOffset);
};

#endif
