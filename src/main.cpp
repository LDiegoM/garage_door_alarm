#include <Arduino.h>

#include <door_device.h>
#include <alarm_device.h>
#include <all_in_one_device.h>

enum deviceType {
    DoorDev,
    AlarmDev,
    AllInOneDev
};

Device *dev = nullptr;

void setup() {
    dev = new DoorDevice();
    // dev = new AlarmDevice();
    // dev = new AllInOneDevice();
    dev->setup();
}

void loop() {
    dev->loop();
}
