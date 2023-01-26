#include <internal/door_device/alarm.h>

//////////////////// Constructor
Alarm::Alarm(DoorStatus *doorStatus, uint8_t buzzerPin) {
    m_doorStatus = doorStatus;
    m_buzzerPin = buzzerPin;

    m_toneTimer = new Timer(TONE_TIME_MILLIS);
    m_silenceTimer = new Timer(SILENCE_TIME_MILLIS);

    m_startAlarmTimer = new Timer(START_ALARM_TIME_SEC * 1000);

    // For ESP32
    // ledcSetup(0, 250, 16);
}

//////////////////// Public methods implementation
void Alarm::loop() {
    m_currentDoorStatus = m_doorStatus->currentStatus();

    if (m_currentDoorStatus == Opened) {
        if (m_toneTimer->isRunning() || m_silenceTimer->isRunning()) {
            sound();
            return;
        }

        if (!m_startAlarmTimer->isRunning())
            m_startAlarmTimer->start();

        if (m_startAlarmTimer->isTime()) {
            m_startAlarmTimer->stop();
            sound();
        }

    } else if (m_currentDoorStatus == Closed) {
        mute();
        m_startAlarmTimer->stop();
    }
}

//////////////////// Private methods implementation
void Alarm::sound() {
    if (!m_toneTimer->isRunning() && !m_silenceTimer->isRunning()) {
        tone(m_buzzerPin, TONE);
        m_toneTimer->start();
    }

    if (m_toneTimer->isTime()) {
        noTone(m_buzzerPin);
        m_toneTimer->stop();
        m_silenceTimer->start();
    }

    if (m_silenceTimer->isTime()) {
        tone(m_buzzerPin, TONE);
        m_toneTimer->start();
        m_silenceTimer->stop();
    }
}

void Alarm::mute() {
    noTone(m_buzzerPin);
    m_toneTimer->stop();
    m_silenceTimer->stop();
}
