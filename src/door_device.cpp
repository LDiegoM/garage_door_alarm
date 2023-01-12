#include <door_device.h>

//////////////////// Constructor
DoorDevice::DoorDevice() {
#if defined(ESP8266) && defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = 3;
#elif defined(ESP8266) && !defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = D5;
#else
    m_serialSpeed = 115200;
    m_irPin = GPIO_NUM_39;
    m_buzzerPin = GPIO_NUM_27;
#endif
}

//////////////////// Public methods implementation
void DoorDevice::setup() {
    Serial.begin(m_serialSpeed);
    // delay(2000);

    Serial.println("Starting DoorDevice setup");

    Serial.println("Creating Sensors object");
    m_sensors = new Sensors(m_irPin);

    Serial.println("Creating DoorStatus object");
    m_doorStat = new DoorStatus(m_sensors);
    m_doorStat->begin();

    Serial.println("Creating Storage object");
    m_storage = new Storage();
    if (!m_storage->begin())
        return;

    Serial.println("Creating Settings object");
    m_settings = new Settings(m_storage);
    if (!m_settings->begin())
        return;
    if (!m_settings->isSettingsOK())
        return;
    settings_t config = m_settings->getSettings();

    Serial.println("Creating WiFiConnection object");
    m_wifi = new WiFiConnection(m_settings);
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

    // pinMode(m_irPin, INPUT);
}

void DoorDevice::loop() {
    m_doorStat->loop();

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
