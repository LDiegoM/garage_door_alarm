#ifndef alarm_h
#define alarm_h

// For ESP32
// #include "esp32-hal-ledc.h"

#include <internal/low_level/timer.h>
#include <internal/door/door_status.h>

class Alarm {
    private:
        DoorStatus *m_doorStatus;
        doorStatus m_currentDoorStatus = Unknown;

        uint8_t m_buzzerPin;
        const unsigned int TONE = 2093;
        const uint16_t TONE_TIME_MILLIS = 250;
        Timer *m_toneTimer;
        const uint16_t SILENCE_TIME_MILLIS = 100;
        Timer *m_silenceTimer;

        const uint16_t START_ALARM_TIME_SEC = 1;
        Timer *m_startAlarmTimer;

        void sound();
        void mute();
    public:
        Alarm(DoorStatus *doorStatus, uint8_t buzzerPin);

        void loop();
};

#endif
