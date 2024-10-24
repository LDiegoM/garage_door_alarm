#include <internal/core/application.h>

//////////////////// Constructor
Application::Application(String name) {
    m_app.name = name;
    m_app.deviceID = getDeviceID();
    lg = new Logging(LOG_LEVEL_WARNING);
}
Application::Application(String name, uint8_t logLevel) {
    m_app.name = name;
    m_app.deviceID = getDeviceID();
    lg = new Logging(logLevel);
}
Application::Application(String name, uint16_t pinBootIndicator) {
    m_app.name = name;
    m_app.deviceID = getDeviceID();
    lg = new Logging(LOG_LEVEL_WARNING);
    m_bootIndicator = new BootIndicator(pinBootIndicator, true);
}
Application::Application(String name, uint16_t pinBootIndicator, uint8_t logLevel) {
    m_app.name = name;
    m_app.deviceID = getDeviceID();
    lg = new Logging(logLevel);
#ifdef ESP8266
    m_bootIndicator = new BootIndicator(pinBootIndicator, true);
#else
    m_bootIndicator = new BootIndicator(pinBootIndicator, true);
#endif
}
Application::~Application() {
    if (m_storage != nullptr) {
        delete m_storage;
        m_storage = nullptr;
    }

    if (m_bootIndicator != nullptr) {
        delete m_bootIndicator;
        m_bootIndicator = nullptr;
    }

    if (m_wifi != nullptr) {
        delete m_wifi;
        m_wifi = nullptr;
    }

    if (m_dateTime != nullptr) {
        delete m_dateTime;
        m_dateTime = nullptr;
    }
    delete lg;
}

//////////////////// Public methods implementation
String Application::name() {
    return m_app.name;
}

void Application::setDeviceID(String deviceID) {
    m_app.deviceID = deviceID;
    if (_mqtt != nullptr)
        _mqtt->setDeviceID(deviceID);
}
String Application::deviceID() {
    return m_app.deviceID;
}
void Application::setGeoLocation(geoLocation_t geoLocation) {
    m_app.geoLocation.lat = geoLocation.lat;
    m_app.geoLocation.lng = geoLocation.lng;
    if (_mqtt != nullptr)
        _mqtt->setLocation(getLocation());

}
void Application::setGeoLocation(String geoLocationLat, String geoLocationLng) {
    m_app.geoLocation.lat = geoLocationLat;
    m_app.geoLocation.lng = geoLocationLng;
    if (_mqtt != nullptr)
        _mqtt->setLocation(getLocation());
}
geoLocation_t Application::geoLocation() {
    return m_app.geoLocation;
}
String Application::getLocation() {
    return String(m_app.geoLocation.lat) + "," + String(m_app.geoLocation.lng);
}

bool Application::beginStorage() {
    if (m_storage == nullptr)
        m_storage = new Storage();
    
    while (!m_storage->begin()) {
        Serial.print(".");
    }
    lg->setStorage(m_storage);
    return true;
}
Storage* Application::storage() {
    return m_storage;
}

BootIndicator* Application::bootIndicator() {
    return m_bootIndicator;
}

void Application::beginWiFi(std::vector<wifiAP_t> wifiAPs) {
    if (m_wifi != nullptr)
        delete m_wifi;
    m_wifi = new WiFiConnection(wifiAPs);
    m_wifi->begin();
}
void Application::beginWiFi(std::vector<wifiAP_t> wifiAPs, String apSSID) {
    if (m_wifi != nullptr)
        delete m_wifi;
    m_wifi = new WiFiConnection(wifiAPs, apSSID);
    m_wifi->begin();
}
void Application::beginWiFi(String apSSID) {
    if (m_wifi != nullptr)
        delete m_wifi;
    m_wifi = new WiFiConnection(apSSID);
    m_wifi->begin();
}
WiFiConnection* Application::wifi() {
    return m_wifi;
}

bool Application::beginDateTime(dateTime_t settings) {
    if (m_dateTime != nullptr)
        delete m_dateTime;
    m_dateTime = new DateTime(settings);
    bool ok = m_dateTime->begin();
    lg->setDateTime(m_dateTime);
    return ok;
}
DateTime* Application::dateTime() {
    return m_dateTime;
}

bool Application::beginMqtt(mqtt_t settings) {
    if (m_wifi == nullptr || settings.server.equals("") || settings.username.equals(""))
        return false;
    if (_mqtt != nullptr)
        free (_mqtt);
    lg->debug("application mqtt begin", __FILE__, __LINE__,
        lg->newTags()
            ->add("server", settings.server + ":" + String(settings.port))
            ->add("username", settings.username)
    );

    _mqtt = new MqttConnection(name(), deviceID(), getLocation(), wifi(), storage(), settings);
    lg->debug("new MqttConnection completed", __FILE__, __LINE__);
    bool ok = _mqtt->begin();
    lg->debug("MqttConnection.begin() completed", __FILE__, __LINE__);
    return ok;
}
MqttConnection* Application::mqtt() {
    return _mqtt;
}

void Application::loop() {
    if (m_bootIndicator != nullptr)
        m_bootIndicator->loop();

    if (lg != nullptr)
        lg->loop();
    
    if (_mqtt != nullptr)
        _mqtt->loop();
}

//////////////////// Private methods implementation
String Application::getDeviceID() {
    // TODO: Implement a default proper device id
    return name() + String("_device");
}
