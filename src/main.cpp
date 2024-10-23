#include <Arduino.h>

#include <internal/core/application.h>

#include <internal/low_level/sensors.h>
#include <internal/low_level/button.h>
#include <internal/settings/settings.h>
#include <handlers/http.h>
#include <handlers/mqtt.h>
#include <internal/door_device/door_status.h>
#include <internal/door_device/door_bell.h>

unsigned long m_serialSpeed;
uint8_t m_irPin;
uint8_t m_buzzerPin;
uint8_t m_bootIndicatorPin;
uint8_t m_buttonPin;

Application *app = nullptr;
Sensors *m_sensors;
DoorStatus *m_doorStat;
Settings *m_settings;
DateTime *m_dateTime;
Button *m_button;
DoorBell *m_doorBell;

void buttonPressed() {
    m_doorBell->ringDoorbell();
}

bool isWiFiConnected() {
    if (app == nullptr)
        return false;
    if (app->wifi() == nullptr)
        return false;
    return app->wifi()->isConnected();
}

void setup() {
#if defined(ESP8266) && defined(ARDUINO_ESP8266_ESP01)
    m_serialSpeed = 9600;
    m_irPin = 3;            // GPIO3 (Rx)
    m_buzzerPin = 1;        // GPIO1 (Tx)
    m_bootIndicatorPin = 2; // GPIO2
    m_buttonPin = 0;        // GPIO0
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

    app = new Application("garage_door", m_bootIndicatorPin, LOG_LEVEL_DEBUG);
    if (!app->beginStorage()) {
        app->bootIndicator()->startErrorBlink();
        return;
    }

    lg->info("device is starting", __FILE__, __LINE__,
        lg->newTags()
            ->add("app_name", app->name())
            ->add("device_id", app->deviceID())
    );

    m_settings = new Settings(app);
    if (!m_settings->begin()) {
        lg->error("Could not load settings", __FILE__, __LINE__);
        app->bootIndicator()->startErrorBlink();
        return;
    }
    if (!m_settings->isSettingsOK()) {
        lg->error("Settings are not ok", __FILE__, __LINE__);
        app->bootIndicator()->startErrorBlink();
        return;
    }
    settings_t config = m_settings->getSettings();
    lg->setLevel(config.logging.level);
    lg->setRefreshPeriod(config.logging.refreshPeriod);
    app->setDeviceID(config.app.deviceID);
    app->setGeoLocation(config.app.geoLocation);

    m_sensors = new Sensors(m_irPin);

    m_doorStat = new DoorStatus(m_sensors);
    m_doorStat->begin();

    m_button = new Button(m_buttonPin, [](){buttonPressed();});
    m_doorBell = new DoorBell();

    app->beginWiFi(config.wifiAPs, app->deviceID());
    app->beginDateTime(config.dateTime);
    app->beginMqtt(config.mqtt.connection);

    httpHandlers = new HttpHandlers(app, m_settings, m_doorStat);
    if (!httpHandlers->begin()) {
        lg->error("Could not start http server", __FILE__, __LINE__);
        app->bootIndicator()->startErrorBlink();
        return;
    }

    if (app->wifi()->isModeAP()) {
        lg->warn("WiFi in AP mode!", __FILE__, __LINE__,
            lg->newTags()
                ->add("ap_name", app->wifi()->getDeviceAPSSID())
        );
        app->bootIndicator()->startWarningBlink();
    } else {
        lg->debug("WiFi OK", __FILE__, __LINE__);
        app->bootIndicator()->setIndicatorStatusCallback([](){
            return !isWiFiConnected();
        });
    }

    mqttHandlers = new MqttHandlers(m_doorStat);
    mqttHandlers->begin();
}

void loop() {
    if (app != nullptr)
        app->loop();

    m_doorStat->loop();
    m_doorBell->loop();

    if (m_settings == nullptr || !m_settings->isSettingsOK())
        return;

    if (httpHandlers != nullptr)
        httpHandlers->loop();

    if (mqttHandlers != nullptr)
        mqttHandlers->loop();

    if (m_button != nullptr)
        m_button->check();
}
