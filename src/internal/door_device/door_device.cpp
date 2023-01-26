#include <internal/door_device/door_device.h>

//////////////////// Constructor
DoorDevice::DoorDevice(bool isBuzzerConnected) {
    m_isBuzzerConnected = isBuzzerConnected;

#if defined(ESP8266) && defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = 3;
    m_buzzerPin = 2;
#elif defined(ESP8266) && !defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = D5;
    m_buzzerPin = D1;
#else
    m_serialSpeed = 115200;
    m_irPin = GPIO_NUM_39;
    m_buzzerPin = GPIO_NUM_27;
#endif
}

//////////////////// Public methods implementation
void DoorDevice::setup() {
    Serial.begin(m_serialSpeed);

    Serial.println("Starting AllInOne setup");

    Serial.println("Creating Sensors object");
    m_sensors = new Sensors(m_irPin);

    Serial.println("Creating DoorStatus object");
    m_doorStat = new DoorStatus(m_sensors);
    m_doorStat->begin();

    if (m_isBuzzerConnected) {
        Serial.println("Creating Alarm object");
        m_garage_alarm = new Alarm(m_doorStat, m_buzzerPin);
    }

    Serial.println("Creating Storage object");
    m_storage = new Storage();
    Serial.println("Begining Storage object");
    if (!m_storage->begin())
        return;

    Serial.println("Creating Settings object");
    m_settings = new Settings(m_storage);
    Serial.println("Begining Settings object");
    if (!m_settings->begin()) {
        Serial.println("Error begining Settings object");
        return;
    }
    if (!m_settings->isSettingsOK()) {
        Serial.println("Settings not ok!!");
        return;
    }
    settings_t config = m_settings->getSettings();

    Serial.println("Creating WiFiConnection object");
    m_wifi = new WiFiConnection(m_settings);
    Serial.println("Begining WiFiConnection object");
    m_wifi->begin();
    Serial.printf("WiFi AP: %s - IP: %s\n", m_wifi->getSSID().c_str(), m_wifi->getIP().c_str());

    // Configure current time
    configTime(config.dateTime.gmtOffset * 60 * 60,
               config.dateTime.daylightOffset * 60 * 60,
               config.dateTime.server.c_str());
    m_dateTime = new DateTime();

    dataLogger = new DataLogger(m_doorStat, m_dateTime, m_storage, config.logger.outputPath);

    mqtt = new MqttHandlers(m_wifi, m_doorStat, m_settings, dataLogger, m_storage);
    if (!mqtt->begin())
        Serial.println("MQTT is not connected");

    httpHandlers = new HttpHandlers(m_wifi, m_storage, m_settings, dataLogger, m_doorStat, mqtt);
    if (!httpHandlers->begin()) {
        Serial.println("Could not start http server");
        return;
    }
}

void DoorDevice::loop() {
    m_doorStat->loop();

    if (m_isBuzzerConnected)
        m_garage_alarm->loop();

    if (!m_settings->isSettingsOK())
        return;

    if (dataLogger != nullptr)
        dataLogger->loop();

    if (httpHandlers != nullptr)
        httpHandlers->loop();

    if (mqtt != nullptr)
        mqtt->loop();
}

//////////////////// Private methods implementation
