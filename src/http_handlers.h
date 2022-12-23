#ifndef http_handlers_h
#define http_handlers_h

#ifdef ESP8266
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WebServer.h>
#else
#error "Unsupported platform"
#endif

#include <ArduinoJson.h>

#include <data_logger.h>
#include <mqtt_handlers.h>
#include <door_status.h>
#include <settings.h>
#include <storage.h>
#include <wifi_connection.h>

struct settings_mqtt_t {
    String server, username, password;
    uint16_t port;
    String certData;
};

struct settings_date_t {
    String server;
    long gmtOffset;
    int daylightOffset;
};

/////////// HTTP Handlers
void downloadLogs();
void deleteLogs();
void restart();
void getStatus();
void getSettings();
void delSettings();

void getBootstrapCSS();
void getBootstrapJS();
void getNotFound();

void getStatusHTML();

void getSettingsWiFi();
void addSettingsWiFi();
void updSettingsWiFi();
void delSettingsWiFi();

void getSettingsMQTT();
void updSettingsMQTT();
void getSettingsMQTTCert();

void getSettingsDate();
void updSettingsDate();

void getAdmin();

class HttpHandlers {
    private:
        const uint16_t METEO_HTTP_PORT = 80;

        const char* BOOTSTRAP_CSS = "https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css";
        const char* BOOTSTRAP_JS = "https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js";

        const char* CONNECTED = "Connected";
        const char* DISCONNECTED = "Disconnected";

        const char* MSG_OK = "ok";
        const char* ERR_GENERIC = "Error saving settings. Please try again";
        const char* ERR_WIFI_AP_NOT_FOUND = "AP ssid not found";
        const char* ERR_WIFI_AP_IS_EMPTY = "AP ssid can't be empty";
        const char* ERR_WIFI_AP_EXISTS = "There's already an AP with the same SSID";

        const char* ERR_MQTT_IS_EMPTY = "MQTT parameters can't be empty";
        
        const char* ERR_DATE_IS_EMPTY = "Date Time parameters can't be empty";

        WiFiConnection *m_wifi;
        Storage *m_storage;
        Settings *m_settings;
#ifdef ESP8266
        ESP8266WebServer *m_server;
#elif defined(ESP32)
        WebServer *m_server;
#endif
        DataLogger *m_dataLogger;
        DoorStatus *m_doorStatus;
        MqttHandlers *m_mqtt;

        void defineRoutes();

        String getHeaderHTML(String section);
        String getFooterHTML(String page, String section);
        
        String getStatusHTMLPage();

        String getSettingsWiFiHTMLPage();
        String getSettingsMQTTHTMLPage();
        String getSettingsDateHTMLPage();

        String getAdminHTMLPage();

        wifiAP_t parseWiFiBody(String body);
        std::vector<wifiAP_t> parseMultiWiFiBody(String body);

        settings_mqtt_t parseMQTTBody(String body);

        settings_date_t parseDateBody(String body);

    public:
        HttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings,
                     DataLogger *dataLogger, DoorStatus *doorStatus, MqttHandlers *mqtt);

        bool begin();
        void loop();

        // HTTP handlers
        void handleDownloadLogs();
        bool handleDeleteLogs();
        void handleRestart();
        void handleGetStatus();
        void handleGetSettings();
        void handleDelSettings();

        void handleGetBootstrapCSS();
        void handleGetBootstrapJS();
        void handleGetNotFound();

        void handleGetStatusHTML();

        void handleGetSettingsWiFi();
        void handleAddSettingsWiFi();
        void handleUpdSettingsWiFi();
        void handleDelSettingsWiFi();

        void handleGetSettingsMQTT();
        void handleUpdSettingsMQTT();
        void handleGetSettingsMQTTCert();

        void handleGetSettingsDate();
        void handleUpdSettingsDate();

        void handleGetAdmin();
};

extern HttpHandlers *httpHandlers;

#endif
