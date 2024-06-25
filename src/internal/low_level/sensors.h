#ifndef sensors_h
#define sensors_h

#include <internal/low_level/timer.h>

class Sensors {
    private:
        uint8_t m_irPin;

    public:
        Sensors(uint8_t irPin);

        bool obstacle();
};

#endif
