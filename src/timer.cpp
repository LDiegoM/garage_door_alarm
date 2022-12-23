/*
    generic timer implementation.

    This class implements a timer that can be checked in all loop() cycles.
    The main idea is to avoid the use of delay() function, starting the timer, and then doing sime action
    when it's definded time was reached.

    It also deals with Arduino's logic to avoid the millis() overflow.

    Diego M. Lopez - 2021 (ldiegom@gmail.com)
*/

#include <timer.h>

Timer::Timer(unsigned long duration) {
    m_duration = duration;
    m_millis_callback = millis;
    m_last_millis = 0;
    m_ulong_max = ULONG_MAX;
    m_reset_gap = 0;
    m_running = false;
}

Timer::Timer(unsigned long duration, unsigned long ulong_max, unsigned long(*millis_callback)()) {
    m_duration = duration;
    m_millis_callback = millis_callback;
    m_last_millis = 0;
    m_ulong_max = ulong_max;
    m_reset_gap = 0;
    m_running = false;
}

void Timer::start() {
    m_reset_gap = 0;
    m_last_millis = m_millis_callback();
    m_running = true;
}

void Timer::stop() {
    m_last_millis = m_millis_callback() - m_last_millis;
    m_running = false;
}

bool Timer::isRunning() {
    return m_running;
}

bool Timer::isTime() {
    if (!m_running) {
        return false;
    }

    unsigned long current = m_millis_callback();

    // When millis() got the maximum unsigned long value, Arduino resets to 0.
    if (m_last_millis > current) {
        // Take as new last tick the difference from last tick to unsigned long maximum value.
        m_reset_gap = m_ulong_max - m_last_millis;
        m_last_millis = 0;
    }

    if (current + m_reset_gap - m_last_millis > m_duration) {
        m_reset_gap = 0;
        m_last_millis = current;
        return true;
    }

    return false;
}

unsigned long Timer::elapsed() {
    if (!m_running) {
        return 0;
    }

    unsigned long current = m_millis_callback();

    if (m_last_millis > current) {
        return (m_ulong_max - m_last_millis) + current;
    }
    
    return current + m_reset_gap - m_last_millis;
}

/*
unsigned long my_millis();

unsigned long current_millis = 0;
const unsigned long MAX_MILLIS = ULONG_MAX;

// my_millis is useful to test counting reset with lows maximum value
unsigned long my_millis() {
  unsigned long now = millis();

  if (now - current_millis >= MAX_MILLIS) {
    current_millis = now;
  }

  return now - current_millis;
}
*/
