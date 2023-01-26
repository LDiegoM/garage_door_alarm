#ifndef data_logger_h
#define data_logger_h

#include <internal/low_level/storage.h>
#include <internal/low_level/timer.h>
#include <internal/net/date_time.h>
#include <internal/common/door_status.h>

class DataLogger {
    private:
        CommonDoorStatus *m_doorStatus;
        DateTime *m_dateTime;
        Storage *m_storage;
        String m_filePath;
        String m_lastLogTime;

        doorStatus m_lastDoorStatus = Unknown;

        bool writeData(doorStatus currentStatus);
    
    public:
        DataLogger(CommonDoorStatus *doorStatus, DateTime *dateTime, Storage *storage, String filePath);

        void loop();
        void logData();
        String getLastLogTime();

        // logSize returns the size of the log file in KB
        String logSize();
};

extern DataLogger *dataLogger;

#endif
