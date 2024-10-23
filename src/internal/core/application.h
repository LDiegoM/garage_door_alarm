#ifndef application_h
#define application_h

#include <Arduino.h>
#include <vector>

#include <internal/core/geo_location.h>
#include <internal/core/boot_indicator.h>
#include <internal/core/storage.h>
#include <internal/core/logging.h>
#include <internal/core/wifi_connection.h>
#include <internal/core/date_time.h>
#include <internal/core/mqtt_connection.h>

struct application_t {
    String name;
    String deviceID;
    geoLocation_t geoLocation;
};

class Application {
    private:
        application_t m_app;
        BootIndicator *m_bootIndicator = nullptr;
        Storage *m_storage = nullptr;
        WiFiConnection *m_wifi = nullptr;
        DateTime *m_dateTime = nullptr;

        String getDeviceID();

    public:
        Application(String name);
        Application(String name, uint8_t logLevel);
        Application(String name, uint16_t pinBootIndicator);
        Application(String name, uint16_t pinBootIndicator, uint8_t logLevel);
        ~Application();

        String name();

        void setDeviceID(String deviceID);
        String deviceID();
        void setGeoLocation(geoLocation_t geoLocation);
        void setGeoLocation(String geoLocationLat, String geoLocationLng);
        geoLocation_t geoLocation();
        String getLocation();

        bool beginStorage();
        Storage* storage();

        BootIndicator* bootIndicator();

        void beginWiFi(std::vector<wifiAP_t> wifiAPs);
        void beginWiFi(std::vector<wifiAP_t> wifiAPs, String apSSID);
        void beginWiFi(String apSSID);
        WiFiConnection* wifi();

        bool beginDateTime(dateTime_t settings);
        DateTime* dateTime();

        bool beginMqtt(mqtt_t settings);
        MqttConnection* mqtt();

        void loop();
};

#endif
