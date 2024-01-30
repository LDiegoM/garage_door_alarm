#ifndef all_in_one_device_h
#define all_in_one_device_h

#include <internal/low_level/data_logger.h>
#include <internal/low_level/sensors.h>
#include <internal/low_level/storage.h>
#include <internal/low_level/timer.h>
#include <internal/low_level/button.h>
#include <internal/net/date_time.h>
#include <internal/net/http_handlers.h>
#include <internal/net/wifi_connection.h>
#include <internal/settings/settings.h>
#include <internal/door_device/door_status.h>
#include <internal/door_device/mqtt_handlers.h>
#include <internal/door_device/alarm.h>
#include <internal/door_device/door_bell.h>

class DoorDevice {
    private:
        unsigned long m_serialSpeed;
        uint8_t m_irPin;
        uint8_t m_buzzerPin;
        uint8_t m_buttonPin;
        bool m_isBuzzerConnected;

        Sensors *m_sensors;
        DoorStatus *m_doorStat;
        Storage *m_storage;
        Settings *m_settings;
        WiFiConnection *m_wifi;
        DateTime *m_dateTime;
        Alarm *m_garage_alarm;
        Button *m_button;
        DoorBell *m_doorBell;

    public:
        DoorDevice(bool isBuzzerConnected);

        void setup();
        void loop();

        void buttonPressed();
};

extern DoorDevice *dev;

#endif
