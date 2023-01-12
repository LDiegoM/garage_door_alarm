#include <internal/door/data_logger.h>

DataLogger *dataLogger = nullptr;

//////////////////// Constructor
DataLogger::DataLogger(DoorStatus *doorStat, DateTime *dateTime, Storage *storage, String filePath) {
    m_doorStatus = doorStat;
    m_dateTime = dateTime;
    m_storage = storage;
    m_filePath = filePath;
    m_lastLogTime = "0000-00-00 00:00:00";
}

//////////////////// Public methods implementation
void DataLogger::loop() {
    doorStatus currentStatus = m_doorStatus->currentStatus();

    if (currentStatus != m_lastDoorStatus)
        writeData(currentStatus);
}

void DataLogger::logData() {
    writeData(m_doorStatus->currentStatus());
}

String DataLogger::getLastLogTime() {
    return m_lastLogTime;
}

String DataLogger::logSize() {
    return String(((float) m_storage->fileSize(m_filePath.c_str())) / 1024) + " kb";
}

//////////////////// Private methods implementation
bool DataLogger::writeData(doorStatus currentStatus) {
    if (!m_dateTime->refresh())
        return false;

    String logTime = m_dateTime->toString() + "\t";
    String fullData = logTime + m_doorStatus->getDoorStatus(currentStatus) + "\n";

    bool res = m_storage->appendFile(m_filePath.c_str(), fullData.c_str());
    if (res) {
        m_lastLogTime = logTime;
        m_lastDoorStatus = currentStatus;
    }

    return res;
}
