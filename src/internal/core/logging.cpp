#include <internal/core/logging.h>

const char* LOGGING_FILE = "/logging/logs.txt";

Logging *lg = nullptr;

//////////////////// Constructor
LogTags::LogTags() {
    m_tags.clear();
}

LogTags::~LogTags() {
    m_tags.clear();
}

Logging::Logging(uint8_t level) {
    m_level = level;
    m_refreshPeriod = 0;
}
Logging::Logging(uint8_t level, Storage *storage) {
    m_level = level;
    m_refreshPeriod = 0;
    m_storage = storage;
}
Logging::Logging(uint8_t level, Storage *storage, DateTime *dt) {
    m_level = level;
    m_refreshPeriod = 0;
    m_storage = storage;
    m_dt = dt;
}

//////////////////// Public methods implementation
LogTags* LogTags::add(String name, String value) {
    logTag_t tag = logTag_t{
        name = name,
        value = value
    };
    m_tags.push_back(tag);
    return this;
}

String LogTags::toString() {
    String s = "";
    for (size_t i = 0; i < m_tags.size(); i++) {
        s += tagToString(m_tags[i]);
    }
    return s;
}

void Logging::setStorage(Storage *storage) {
    m_storage = storage;
}

void Logging::setLevel(uint8_t level) {
    m_level = level;
}
uint8_t Logging::getLevel() {
    return m_level;
}
void Logging::setRefreshPeriod(uint16_t refreshPeriod) {
    m_refreshPeriod = refreshPeriod;

    if (m_refreshTimer != nullptr) {
        m_refreshTimer->stop();
        delete m_refreshTimer;
    }

    if (m_refreshPeriod != 0) {
        // m_refreshTimer is defined in hours
        m_refreshTimer = new Timer(m_refreshPeriod * 60 * 60 * 1000);
        m_refreshTimer->start();
    }
}
uint16_t Logging::getRefreshPeriod() {
    return m_refreshPeriod;
}
void Logging::setDateTime(DateTime *dt) {
    m_dt = dt;
}

LogTags* Logging::newTags() {
    return new LogTags();
}

void Logging::debug(String msg, String file, int line) {
    if (m_level > LOG_LEVEL_DEBUG)
        return;
    writeData(getFullData(msg, LOG_LEVEL_DEBUG, file, line));
}
void Logging::info(String msg, String file, int line) {
    if (m_level > LOG_LEVEL_INFO)
        return;
    writeData(getFullData(msg, LOG_LEVEL_INFO, file, line));
}
void Logging::warn(String msg, String file, int line) {
    if (m_level > LOG_LEVEL_WARNING)
        return;
    writeData(getFullData(msg, LOG_LEVEL_WARNING, file, line));
}
void Logging::error(String msg, String file, int line) {
    writeData(getFullData(msg, LOG_LEVEL_ERROR, file, line));
}

void Logging::debug(String msg, String file, int line, LogTags *tags) {
    if (m_level > LOG_LEVEL_DEBUG) {
        delete tags;
        return;
    }
    writeData(getFullData(msg, LOG_LEVEL_DEBUG, file, line, tags));
}
void Logging::info(String msg, String file, int line, LogTags *tags) {
    if (m_level > LOG_LEVEL_INFO) {
        delete tags;
        return;
    }
    writeData(getFullData(msg, LOG_LEVEL_INFO, file, line, tags));
}
void Logging::warn(String msg, String file, int line, LogTags *tags) {
    if (m_level > LOG_LEVEL_WARNING) {
        delete tags;
        return;
    }
    writeData(getFullData(msg, LOG_LEVEL_WARNING, file, line, tags));
}
void Logging::error(String msg, String file, int line, LogTags *tags) {
    writeData(getFullData(msg, LOG_LEVEL_ERROR, file, line, tags));
}

String Logging::logSize() {
    if (m_storage == nullptr)
        return String("0 kb");

    return String(((float) m_storage->fileSize(LOGGING_FILE)) / 1024) + " kb";
}
bool Logging::clear() {
    if (m_storage == nullptr)
        return false;

    return m_storage->remove(LOGGING_FILE);
}

void Logging::loop() {
    if (m_refreshTimer == nullptr || m_storage == nullptr)
        return;

    if (!m_refreshTimer->isTime())
        return;

    if (m_storage->remove(LOGGING_FILE))
        lg->info("logging file was deleted by periodical timer", __FILE__, __LINE__, lg->newTags()->add("period", String(m_refreshPeriod) + " hours"));
    else
        lg->error("periodical timer could not delete logging file", __FILE__, __LINE__);
}

//////////////////// Private methods implementation
String LogTags::tagToString(logTag_t tag) {
    return String("[") + tag.name + ":" + tag.value + "]";
}

String Logging::getDateTime() {
    String s = "[dt:";

    if (m_dt != nullptr) {
        if (!m_dt->refresh())
            s += "ERR";
        else
            s += m_dt->toString();
    } else {
        s += "N/A";
    }
    return s + "]";
}

String Logging::getLogLevel(uint8_t level) {
    String s = "[level:";
    switch (level) {
    case LOG_LEVEL_DEBUG:
        s += "debug";
        break;
    case LOG_LEVEL_INFO:
        s += "info";
        break;
    case LOG_LEVEL_WARNING:
        s += "warn";
        break;
    case LOG_LEVEL_ERROR:
        s += "error";
        break;
    default:
        s += "N/A";
        break;
    }
    return s + "]";
}

String Logging::getFullData(String msg, uint8_t level, String file, int line) {
    return getDateTime() + getLogLevel(level) + "[" + file + ":" + String(line) + "][msg:" + msg + "][free_ram:" + getFreeMem() + "]";
}
String Logging::getFullData(String msg, uint8_t level, String file, int line, LogTags *tags) {
    String sTags = tags->toString();
    delete tags;
    return getFullData(msg, level, file, line) + sTags;
}

void Logging::writeData(String fullData) {
    String data = fullData + "\n";
    //if (m_storage != nullptr)
    //    m_storage->appendFile(LOGGING_FILE, data.c_str());
    //else
        Serial.print(data);
}

String Logging::getFreeMem() {
    return String((float) ESP.getFreeHeap() / 1024) + " kb";
}
