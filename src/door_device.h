#ifndef door_device_h
#define door_device_h

#include <device.h>

#include <internal/low_level/sensors.h>
#include <internal/low_level/storage.h>
#include <internal/low_level/timer.h>
#include <internal/net/date_time.h>
#include <internal/net/http_handlers.h>
#include <internal/net/mqtt_handlers.h>
#include <internal/net/wifi_connection.h>
#include <internal/settings/settings.h>
#include <internal/door/data_logger.h>
#include <internal/door/door_status.h>

class DoorDevice : public Device {
    private:
        unsigned long m_serialSpeed;
        uint8_t m_irPin;

        Sensors *m_sensors;
        DoorStatus *m_doorStat;
        Storage *m_storage;
        Settings *m_settings;
        WiFiConnection *m_wifi;
        DateTime *m_dateTime;

    public:
        DoorDevice();

        void setup();
        void loop();
};

#endif
