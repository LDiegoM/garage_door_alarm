#include <Arduino.h>

#include <internal/alarm_device/alarm_device.h>

AlarmDevice *dev = nullptr;

void setup() {
    dev = new AlarmDevice();
    dev->setup();
}

void loop() {
    dev->loop();
}
