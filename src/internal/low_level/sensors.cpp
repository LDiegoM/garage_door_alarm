#include <internal/low_level/sensors.h>

//////////////////// Constructor
Sensors::Sensors(uint8_t irPin) {
    m_irPin = irPin;
    pinMode(m_irPin, INPUT);
}

//////////////////// Public methods implementation
bool Sensors::obstacle() {
    int value = digitalRead(m_irPin);
    return (value == LOW);
}

//////////////////// Private methods implementation
