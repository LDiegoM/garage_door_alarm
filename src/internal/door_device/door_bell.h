#ifndef door_bell_h
#define door_bell_h

#include <internal/core/mqtt_connection.h>
#include <internal/core/timer.h>

class DoorBell {
    private:
        int8_t m_soundDurationSeconds;
        const uint8_t DEFAULT_SOUND_DURATION_SECONDS = 7;
        Timer *m_tmrRingWait = nullptr;

        const char* TOPIC_DOOR_BELL = "topic-garage-door";

    public:
        DoorBell();

        bool ringDoorbell();
        void loop();
};

#endif
