/*
https://github.com/siara-cc/esp32_arduino_sqlite3_lib/blob/master/examples/sqlite3_littlefs/sqlite3_littlefs.ino

Table: events
Fields:
    id                  int PK  auto_increment
    time                datetime
    status              varchar(11) -- door opened -- door closed
    door_closed         int
    last_duration_sec   bigint #10 days = 864000 sec -- 1 day = 86400

CREATE TABLE events(
   id                INTEGER      PRIMARY KEY AUTOINCREMENT,
   time              TEXT         NOT NULL,
   status            VARCHAR(11)  NOT NULL,
   door_closed       BOOLEAN      NOT NULL,
   last_duration_sec INTEGER      NOT NULL
);
*/
#if !defined(database_h) && defined(ESP32)
#define database_h

#include <sqlite3.h>

#include <internal/low_level/storage.h>
#include <internal/net/date_time.h>
#include <internal/common/door_status.h>

class Database {
    private:
        Storage *m_storage;
        String m_dbPath;
        sqlite3 *m_db = nullptr;
        const char *STORAGE_TYPE = "littlefs";

        char *m_errMsg = 0;

    public:
        Database(Storage *storage, String dbPath);

        bool begin();
        /*
         * The openDatabase() routine will try to open an existing database.
         * If file doesn't exists, will return false so client can call beginDatabase()
         * routine, passing as argument the full CREATE sentence.
        */
        bool openDatabase();
        bool closeDatabase();
        bool createDatabase(String createSQL);
        bool execNonQuerySQL(String sql);

        /* Theese methos will be part of statistics class as part of the alarm_device
        bool addEvent(doorStatus status);
        int getMaxOpenedTimeSinceDays(int days);
        int getMaxOpenedTimeSinceMonths(int days);*/
};

#endif
