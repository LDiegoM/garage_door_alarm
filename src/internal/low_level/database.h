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
#ifndef database_h
#define database_h

#ifndef ESP32
#error "Unsupported platform. Only ESP32 permitted for Database implementation."
#endif

#include <sqlite3.h>

#include <internal/low_level/storage.h>
#include <internal/net/date_time.h>
#include <internal/common/door_status.h>

class Database {
    private:
        Storage *m_storage;

    public:
        Database(Storage *storage);

        bool begin();
        bool addEvent(doorStatus status);
        int getMaxOpenedTimeSinceDays(int days);
        int getMaxOpenedTimeSinceMonths(int days);
};

#endif
