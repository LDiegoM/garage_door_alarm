#ifndef boot_indicator_h
#define boot_indicator_h

#include <internal/low_level/timer.h>

class BootIndicator {
    private:
        const unsigned long BOOT_INDICATOR_ERROR = 100;
        const unsigned long BOOT_INDICATOR_WARNING = 800;

        uint8_t m_pinBootIndicator;
        bool m_indicatorIsOn = false;
        bool m_invertLevels = false;
        Timer *m_tmrBlink = nullptr;
        bool(*indicatorCallback)() = nullptr;

        int indicatorLevel();
        void startBlink(unsigned long millis);

    public:
        BootIndicator(uint8_t pinBootIndicator, bool indicatorStartsOn = false, bool invertLevels = false);

        void alternateOnOff();
        void turnOn();
        void turnOff();
        void startErrorBlink();
        void startWarningBlink();
        void stopBlink();
        void setIndicatorStatusCallback(bool(*callback)());
        void loop();
};

#endif
