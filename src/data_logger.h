#ifndef data_logger_h
#define data_logger_h

#include <door_status.h>
#include <date_time.h>
#include <storage.h>
#include <timer.h>

class DataLogger {
    private:
        DoorStatus *m_doorStatus;
        DateTime *m_dateTime;
        Storage *m_storage;
        String m_filePath;
        String m_lastLogTime;

        doorStatus m_lastDoorStatus = Unknown;

        bool writeData(doorStatus currentStatus);
    
    public:
        DataLogger(DoorStatus *doorStatus, DateTime *dateTime, Storage *storage, String filePath);

        void loop();
        void logData();
        String getLastLogTime();

        // logSize returns the size of the log file in KB
        String logSize();
};

extern DataLogger *dataLogger;

#endif
