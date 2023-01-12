#include <Arduino.h>

#include <internal/door_device/door_device.h>

DoorDevice *dev = nullptr;

void setup() {
    dev = new DoorDevice(true);
    dev->setup();
}

void loop() {
    dev->loop();
}
