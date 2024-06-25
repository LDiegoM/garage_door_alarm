#ifndef common_door_status_h
#define common_door_status_h

#include <Arduino.h>

enum doorStatus {
    Unknown,
    Opened,
    Closed
};

class CommonDoorStatus {
    public:
        virtual doorStatus currentStatus() = 0;
        virtual String getCurrentDoorStatus() = 0;
        virtual String getDoorStatus(doorStatus someStatus) = 0;
};

#endif
