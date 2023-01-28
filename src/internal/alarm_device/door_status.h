#ifndef door_status_h
#define door_status_h

#include <internal/common/door_status.h>

#include <internal/low_level/timer.h>
#include <internal/alarm_device/mqtt_handlers.h>

class AlarmDoorStatus : public CommonDoorStatus {
    private:
        AlarmMqttHandlers *m_mqttHandlers;

        Timer *m_tmrRefreshValues;
        const uint16_t REFRESH_TIME_MILLIS = 250;

        void readValues();

    public:
        AlarmDoorStatus(AlarmMqttHandlers *mqttHandlers);

        void begin();
        void stop();
        void loop();
        doorStatus currentStatus();
        String getCurrentDoorStatus();
        String getDoorStatus(doorStatus someStatus);

};

#endif
