#include <Arduino.h>

#include <internal/door_device/door_device.h>

void setup() {
    dev = new DoorDevice(true);
    dev->setup();
}

void loop() {
    dev->loop();
}
