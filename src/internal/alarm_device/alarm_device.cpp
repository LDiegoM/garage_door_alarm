#ifdef ESP32

#include <internal/alarm_device/alarm_device.h>

//////////////////// Constructor
AlarmDevice::AlarmDevice() {

}

//////////////////// Public methods implementation
void AlarmDevice::setup() {
    Serial.begin(115200);
    sqlite3 *db1;

    Serial.println("Creating Storage object");
    m_storage = new Storage();
    Serial.println("Begining Storage object");
    if (!m_storage->begin()) {
        Serial.println("Failed to load Storage object");
        return;
    }

    m_database = new Database(m_storage);
   // remove existing file
   m_storage->remove("/db/test1.db");

}

void AlarmDevice::loop() {
}

//////////////////// Private methods implementation

#endif
