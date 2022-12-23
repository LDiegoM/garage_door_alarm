#ifndef door_status_h
#define door_status_h

#include <sensors.h>
#include <timer.h>

enum doorStatus {
    Unknown,
    Opened,
    Closed
};

class DoorStatus {
    private:
        Sensors *m_sensors;

        Timer *m_tmrRefreshValues;
        const uint16_t REFRESH_TIME_MILLIS = 250;

        bool m_currentObstacle;

        void readValues();

    public:
        DoorStatus(Sensors *sensors);

        void begin();
        void stop();
        void loop();
        doorStatus currentStatus();
        String getCurrentDoorStatus();
        String getDoorStatus(doorStatus someStatus);

};

#endif
