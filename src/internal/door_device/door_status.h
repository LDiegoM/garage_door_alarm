#ifndef door_status_h
#define door_status_h

#include <internal/common/door_status.h>

#include <internal/low_level/sensors.h>
#include <internal/low_level/timer.h>

class DoorStatus : public CommonDoorStatus {
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
