#include <internal/door_device/door_bell.h>

//////////////////// Constructor
DoorBell::DoorBell() {
    m_soundDurationSeconds = DEFAULT_SOUND_DURATION_SECONDS;
    m_tmrRingWait = new Timer(m_soundDurationSeconds * 1000);
}

//////////////////// Public methods implementation
bool DoorBell::ringDoorbell() {
    if (m_tmrRingWait != nullptr && m_tmrRingWait->isRunning() && !m_tmrRingWait->isTime())
        return false;

    if (mqtt == nullptr || !mqtt->isConnected())
        return false;

    if (!mqtt->publish(TOPIC_DOOR_BELL, "RING"))
        return false;

    m_tmrRingWait->start();
    return true;
}

void DoorBell::loop() {
    if (m_tmrRingWait != nullptr && m_tmrRingWait->isTime())
        m_tmrRingWait->stop();
}

//////////////////// Private methods implementation
