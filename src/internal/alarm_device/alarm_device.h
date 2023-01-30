#if !defined(alarm_device_h) && defined(ESP32)
#define alarm_device_h

/*
    Alarm device must detect door opened or closed condition from mqtt queue suscription.
    And ide for multiple main functions in platformio:
    https://community.platformio.org/t/here-is-a-simple-way-to-have-many-small-main-apps/26009
*/
#include <sqlite3.h>

#include <internal/low_level/storage.h>
#include <internal/low_level/timer.h>
#include <internal/low_level/database.h>
#include <internal/net/date_time.h>
#include <internal/net/wifi_connection.h>
#include <internal/settings/settings.h>

#include <internal/alarm_device/http_handlers.h>

class AlarmDevice {
    private:
        Storage *m_storage;
        Settings *m_settings;
        WiFiConnection *m_wifi;
        DateTime *m_dateTime;
        Database *m_database;
        AlarmHttpHandlers *m_httpHandlers;

    public:
        AlarmDevice();

        void setup();
        void loop();
};

#endif
