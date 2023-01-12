#include <all_in_one_device.h>

//////////////////// Constructor
AllInOneDevice::AllInOneDevice() {
#ifdef ESP8266
    m_serialSpeed = 9600;
    m_irPin = 0;
    m_buzzerPin = 2;
#else
    m_serialSpeed = 115200;
    m_irPin = GPIO_NUM_39;
    m_buzzerPin = GPIO_NUM_27;
#endif
}

//////////////////// Public methods implementation
void AllInOneDevice::setup() {
    Serial.begin(m_serialSpeed);
    delay(2000);

    Serial.println("Starting AllInOne setup");

    Serial.println("Creating Sensors object");
    m_sensors = new Sensors(m_irPin);

    Serial.println("Creating DoorStatus object");
    m_doorStat = new DoorStatus(m_sensors);
    m_doorStat->begin();

    Serial.println("Creating Alarm object");
    m_garage_alarm = new Alarm(m_doorStat, m_buzzerPin);

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

void AllInOneDevice::loop() {
    m_doorStat->loop();
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
