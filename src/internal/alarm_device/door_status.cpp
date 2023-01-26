#include <internal/alarm_device/door_status.h>

//////////////////// Constructor
AlarmDoorStatus::AlarmDoorStatus(AlarmMqttHandlers *mqttHandlers) {
    m_mqttHandlers = mqttHandlers;

    m_tmrRefreshValues = new Timer(REFRESH_TIME_MILLIS);
}

//////////////////// Public methods implementation
void AlarmDoorStatus::begin() {
    m_tmrRefreshValues->start();
}

void AlarmDoorStatus::stop() {
    m_tmrRefreshValues->stop();
}

void AlarmDoorStatus::loop() {
    if (!m_tmrRefreshValues->isRunning())
        return;

    if (m_tmrRefreshValues->isTime())
        readValues();
}

doorStatus AlarmDoorStatus::currentStatus() {
    // FIXME: fix this
    return Unknown; // m_currentObstacle ? Closed : Opened;
}

String AlarmDoorStatus::getCurrentDoorStatus() {
    return getDoorStatus(currentStatus());
}

String AlarmDoorStatus::getDoorStatus(doorStatus someStatus) {
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
void AlarmDoorStatus::readValues() {
    // FIXME: m_currentObstacle = m_sensors->obstacle();
}
