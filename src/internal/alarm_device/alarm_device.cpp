#ifdef ESP32

#include <internal/alarm_device/alarm_device.h>

String createStatisticsTable = "CREATE TABLE events(\
   id                INTEGER      PRIMARY KEY AUTOINCREMENT,\
   time              TEXT         NOT NULL,\
   status            VARCHAR(11)  NOT NULL,\
   door_closed       BOOLEAN      NOT NULL,\
   last_duration_sec INTEGER      NOT NULL\
);";

//////////////////// Constructor
AlarmDevice::AlarmDevice() {

}

//////////////////// Public methods implementation
void AlarmDevice::setup() {
    Serial.begin(115200);

    Serial.println("Starting AlarmDevice setup");

    Serial.println("Creating Storage object");
    m_storage = new Storage();
    Serial.println("Begining Storage object");
    if (!m_storage->begin()) {
        Serial.println("Failed to load Storage object");
        return;
    }

    Serial.println("Creating Settings object");
    m_settings = new Settings(m_storage);
    Serial.println("Begining Settings object");
    if (!m_settings->begin()) {
        Serial.println("Error begining Settings object");
        return;
    }
    if (!m_settings->isSettingsOK()) {
        Serial.println("Settings not ok!!");
        return;
    }
    settings_t config = m_settings->getSettings();

    Serial.println("Creating WiFiConnection object");
    m_wifi = new WiFiConnection(m_settings);
    Serial.println("Begining WiFiConnection object");
    m_wifi->begin();
    Serial.printf("WiFi AP: %s - IP: %s\n", m_wifi->getSSID().c_str(), m_wifi->getIP().c_str());

    // Configure current time
    configTime(config.dateTime.gmtOffset * 60 * 60,
               config.dateTime.daylightOffset * 60 * 60,
               config.dateTime.server.c_str());
    m_dateTime = new DateTime();

    Serial.println("Creating Database object");
    m_database = new Database(m_storage, "/db/statistics.db");
    Serial.println("Begining Database object");
    if (!m_database->begin()) {
        Serial.println("Failed to load Database object");
        return;
    }

    if (!m_database->openDatabase()) {
        Serial.println("Couldn't open Database. Trying to create");
        if (!m_database->createDatabase(createStatisticsTable)) {
            Serial.printf("Couldn't create Database. %s\n", m_database->getErrorMessage());
            return;
        }
    }

    String insertStmt = "INSERT INTO events (time, status, door_closed, last_duration_sec) VALUES (\
    '2023-01-30 00:30:00', 'door opened', false, 0)";
    Serial.println("Try to insert first row");
    if (!m_database->execNonQuerySQL(insertStmt)) {
        Serial.println("Error inserting first row: " + m_database->getErrorMessage());
    }

    Serial.println("Creating HttpHandlers object");
    alarmHttpHandlers = new AlarmHttpHandlers(m_wifi, m_storage, m_settings);
    if (!alarmHttpHandlers->begin()) {
        Serial.println("Could not start http server");
        return;
    }

}

int i = 0;
void AlarmDevice::loop() {
    if (i < 5) {
        Serial.printf("WiFi Status: %s - AP: %s - IP: %s\n", m_wifi->isConnected()?"connected":"disconnected", m_wifi->getSSID().c_str(), m_wifi->getIP().c_str());
        i++;
        if (alarmHttpHandlers != nullptr)
            Serial.println("alarmHttpHandlers not NULL");
    }
    if (alarmHttpHandlers != nullptr)
        alarmHttpHandlers->loop();
}

//////////////////// Private methods implementation

#endif
