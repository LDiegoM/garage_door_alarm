#ifndef logging_h
#define logging_h

#include <vector>
#include <internal/core/storage.h>
#include <internal/core/date_time.h>
#include <internal/core/logging.h>
#include <internal/core/timer.h>

#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR   3

extern const char* LOGGING_FILE;

struct logging_t {
    uint8_t level;
    uint16_t refreshPeriod;
};

struct logTag_t {
    String name;
    String value;
};

class LogTags {
    private:
        std::vector<logTag_t> m_tags;

        String tagToString(logTag_t tag);

    public:
        LogTags();
        ~LogTags();

        LogTags* add(String name, String value);
        String toString();
};

class Logging {
    private:
        uint8_t m_level;
        // refreshPeriod is defined in hours
        uint16_t m_refreshPeriod;
        Storage *m_storage = nullptr;
        DateTime *m_dt = nullptr;
        Timer *m_refreshTimer = nullptr;

        String getDateTime();
        String getLogLevel(uint8_t level);
        String getFullData(String msg, uint8_t level, String file, int line);
        String getFullData(String msg, uint8_t level, String file, int line, LogTags *tags);
        void writeData(String fullData);
        String getFreeMem();

    public:
        Logging(uint8_t level);
        Logging(uint8_t level, Storage *storage);
        Logging(uint8_t level, Storage *storage, DateTime *dt);

        void setStorage(Storage *storage);
        void setLevel(uint8_t level);
        uint8_t getLevel();
        void setRefreshPeriod(uint16_t refreshPeriod);
        uint16_t getRefreshPeriod();
        void setDateTime(DateTime *dt);

        LogTags* newTags();

        void debug(String msg, String file, int line);
        void info(String msg, String file, int line);
        void warn(String msg, String file, int line);
        void error(String msg, String file, int line);

        void debug(String msg, String file, int line, LogTags *tags);
        void info(String msg, String file, int line, LogTags *tags);
        void warn(String msg, String file, int line, LogTags *tags);
        void error(String msg, String file, int line, LogTags *tags);

        String logSize();
        bool clear();

        void loop();
};

extern Logging *lg;

#endif
