#ifndef timer_h
#define timer_h

#include <Arduino.h>
#include <limits.h>

class Timer {
    private:
        unsigned long(*m_millis_callback)();
        unsigned long m_duration, m_last_millis, m_ulong_max, m_reset_gap;
        bool m_running;
    
    public:
        Timer(unsigned long duration);
        Timer(unsigned long duration, unsigned long ulong_max, unsigned long(*millis_callback)());

        void start();
        void stop();
        bool isRunning();
        bool isTime();
        unsigned long elapsed();
};

#endif
