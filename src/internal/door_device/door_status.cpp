#include <internal/door_device/door_status.h>

//////////////////// Constructor
DoorStatus::DoorStatus(Sensors *sensors) {
    m_sensors = sensors;

    m_tmrRefreshValues = new Timer(REFRESH_TIME_MILLIS);
}

//////////////////// Public methods implementation
void DoorStatus::begin() {
    m_tmrRefreshValues->start();
}

void DoorStatus::stop() {
    m_tmrRefreshValues->stop();
}

void DoorStatus::loop() {
    if (!m_tmrRefreshValues->isRunning())
        return;

    if (m_tmrRefreshValues->isTime())
        readValues();
}

doorStatus DoorStatus::currentStatus() {
    return m_currentObstacle ? Closed : Opened;
}

String DoorStatus::getCurrentDoorStatus() {
    return getDoorStatus(currentStatus());
}

String DoorStatus::getDoorStatus(doorStatus someStatus) {
    String ret = "door ";
    switch (someStatus) {
        case Unknown:
            ret += "unknown";
            break;
        case Closed:
            ret += "closed";
            break;
        case Opened:
            ret += "opened";
            break;
    }
    return ret;
}

//////////////////// Private methods implementation
void DoorStatus::readValues() {
    m_currentObstacle = m_sensors->obstacle();
}
