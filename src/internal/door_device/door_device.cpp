#include <internal/door_device/door_device.h>

DoorDevice *dev = nullptr;

//////////////////// Constructor
DoorDevice::DoorDevice(bool isBuzzerConnected) {
    m_isBuzzerConnected = isBuzzerConnected;

#if defined(ESP8266) && defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = 3;
    m_buzzerPin = 1;
    m_bootIndicatorPin = 2;
    m_buttonPin = 0;
#elif defined(ESP8266) && !defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = D5;
    m_buzzerPin = D1;
    m_bootIndicatorPin = 2;
    m_buttonPin = 0;
#else
    m_serialSpeed = 115200;
    m_irPin = GPIO_NUM_39;
    m_buzzerPin = GPIO_NUM_27;
    m_bootIndicatorPin = 2;
    m_buttonPin = 0;
#endif
}

//////////////////// Public methods implementation
void DoorDevice::setup() {
    m_bootIndicator = new BootIndicator(m_bootIndicatorPin, true);
    m_sensors = new Sensors(m_irPin);

    m_doorStat = new DoorStatus(m_sensors);
    m_doorStat->begin();

    if (m_isBuzzerConnected) {
        m_garage_alarm = new Alarm(m_doorStat, m_buzzerPin);
    }

    m_button = new Button(m_buttonPin, [](){
        dev->buttonPressed();
    });
    m_doorBell = new DoorBell();

    m_storage = new Storage();
    if (!m_storage->begin()) {
        m_bootIndicator->startErrorBlink();
        return;
    }

    m_settings = new Settings(m_storage);
    if (!m_settings->begin()) {
        m_bootIndicator->startErrorBlink();
        return;
    }
    if (!m_settings->isSettingsOK()) {
        m_bootIndicator->startErrorBlink();
        return;
    }
    settings_t config = m_settings->getSettings();

    m_wifi = new WiFiConnection(m_settings);
    m_wifi->begin();
    if (m_wifi->isModeAP())
        m_bootIndicator->startWarningBlink();
    else
        m_bootIndicator->setIndicatorStatusCallback([](){
            return !dev->isWiFiConnected();
        });

    // Configure current time
    configTime(config.dateTime.gmtOffset * 60 * 60,
               config.dateTime.daylightOffset * 60 * 60,
               config.dateTime.server.c_str());
    m_dateTime = new DateTime();

    dataLogger = new DataLogger(m_doorStat, m_dateTime, m_storage, config.logger.outputPath);

    mqtt = new MqttHandlers(m_wifi, m_doorStat, m_settings, dataLogger, m_storage);
    mqtt->begin();

    httpHandlers = new HttpHandlers(m_wifi, m_storage, m_settings, dataLogger, m_doorStat, mqtt);
    httpHandlers->begin();
}

void DoorDevice::loop() {
    m_bootIndicator->loop();
    m_doorStat->loop();
    m_doorBell->loop();

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

    if (m_button != nullptr)
        m_button->check();
}

void DoorDevice::buttonPressed() {
    m_doorBell->ringDoorbell();
}

bool DoorDevice::isWiFiConnected() {
    if (m_wifi == nullptr)
        return false;
    return m_wifi->isConnected();
}

//////////////////// Private methods implementation
