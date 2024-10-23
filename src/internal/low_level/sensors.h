#ifndef sensors_h
#define sensors_h

#include <internal/core/timer.h>

class Sensors {
    private:
        uint8_t m_irPin;

    public:
        Sensors(uint8_t irPin);

        bool obstacle();
};

#endif
