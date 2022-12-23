#include <Arduino.h>

#include <data_logger.h>
#include <date_time.h>
#include <http_handlers.h>
#include <mqtt_handlers.h>
#include <sensors.h>
#include <door_status.h>
#include <alarm.h>
#include <settings.h>
#include <storage.h>
#include <timer.h>
#include <wifi_connection.h>

uint8_t irPin;
uint8_t buzzerPin;

Sensors *sensors;
DoorStatus *doorStat;
Alarm *garage_alarm;
Storage *storage;
Settings *settings;
WiFiConnection *wifi;
DateTime *dateTime;

void setup() {
#ifdef ESP8266
    Serial.begin(9600);
    irPin = 0;
    buzzerPin = 2;
#else
    Serial.begin(115200);
    irPin = GPIO_NUM_39;
    buzzerPin = GPIO_NUM_27;
#endif
    delay(2000);

    Serial.println("starting setup");

    Serial.println("Creating Sensors object");
    sensors = new Sensors(irPin);

    Serial.println("Creating DoorStatus object");
    doorStat = new DoorStatus(sensors);
    doorStat->begin();

    Serial.println("Creating Alarm object");
    garage_alarm = new Alarm(doorStat, buzzerPin);

    storage = new Storage();
    if (!storage->begin())
        return;

    settings = new Settings(storage);
    if (!settings->begin())
        return;
    if (!settings->isSettingsOK())
        return;
    settings_t config = settings->getSettings();

    wifi = new WiFiConnection(settings);
    wifi->begin();
    Serial.printf("WiFi AP: %s - IP: %s\n", wifi->getSSID().c_str(), wifi->getIP().c_str());

    // Configure current time
    configTime(config.dateTime.gmtOffset * 60 * 60,
               config.dateTime.daylightOffset * 60 * 60,
               config.dateTime.server.c_str());
    dateTime = new DateTime();

    dataLogger = new DataLogger(doorStat, dateTime, storage, config.logger.outputPath);

    mqtt = new MqttHandlers(wifi, doorStat, settings, dataLogger, storage);
    if (!mqtt->begin())
        Serial.println("MQTT is not connected");

    httpHandlers = new HttpHandlers(wifi, storage, settings, dataLogger, doorStat, mqtt);
    if (!httpHandlers->begin()) {
        Serial.println("Could not start http server");
        return;
    }
}

void loop() {
    doorStat->loop();
    garage_alarm->loop();

    if (!settings->isSettingsOK())
        return;

    if (dataLogger != nullptr)
        dataLogger->loop();

    if (httpHandlers != nullptr)
        httpHandlers->loop();

    if (mqtt != nullptr)
        mqtt->loop();
}
