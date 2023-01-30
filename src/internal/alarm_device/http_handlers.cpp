#include <internal/alarm_device/http_handlers.h>

AlarmHttpHandlers *alarmHttpHandlers = nullptr;

//////////////////// HTTP Handlers
void downloadDB(void) {
    alarmHttpHandlers->handleDownloadDB();
}
void deleteDB(void) {
    alarmHttpHandlers->handleDeleteDB();
    // TODO: Reset device
}

//////////////////// Constructor
AlarmHttpHandlers::AlarmHttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings) :
    HttpHandlers(wifi, storage, settings, NULL, NULL, NULL) {
    
    httpHandlers = this;
}

//////////////////// Public methods implementation
bool AlarmHttpHandlers::begin() {
    Serial.println("AlarmHttpHandlers::begin() - try to begin parent object");
    HttpHandlers::begin();
    Serial.println("AlarmHttpHandlers::begin() - parent object begin ok");

    Serial.println("AlarmHttpHandlers::begin() - try to add db handlers");
    m_server->on("/db", HTTP_GET, downloadDB);
    m_server->on("/db", HTTP_DELETE, deleteDB);
    Serial.println("AlarmHttpHandlers::begin() - db handlers ok");
    return true;
}

/////////// HTTP Handlers
void AlarmHttpHandlers::handleDownloadDB() {
    if (!m_storage->exists("/db/statistics.db")) {
        m_server->send(404, "text/plain", "not found");
        return;
    }

    File file = m_storage->open("/db/statistics.db");
    if (!file) {
        m_server->send(500, "text/plain", "fail to open log file");
        return;
    }

    String dataType = "application/octet-stream";

    m_server->sendHeader("Content-Disposition", "inline; filename=garage_door_statistics.db");

    if (m_server->streamFile(file, dataType) != file.size())
        Serial.println("Sent different data length than expected");
    
    file.close();
}
bool AlarmHttpHandlers::handleDeleteDB() {
    if (!m_storage->exists("/db/statistics.db")) {
        m_server->send(404, "text/plain", "not found");
        return false;
    }

    bool flgOK = m_storage->remove("/db/statistics.db");
    if (flgOK)
        m_server->send(204);
    else
        m_server->send(500, "text/plain", "could not delete file");
    
    return flgOK;
}

//////////////////// Private methods implementation
void AlarmHttpHandlers::defineRoutes() {
    HttpHandlers::defineRoutes();
}
