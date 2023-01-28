#ifdef ESP32

#include <internal/low_level/database.h>

//////////////////// Constructor
Database::Database(Storage *storage, String dbPath) {
    m_storage = storage;
    m_dbPath = dbPath;
}

//////////////////// Public methods implementation
bool Database::begin() {
    return sqlite3_initialize() == SQLITE_OK;
}

bool Database::openDatabase() {
    if (!m_storage->exists(m_dbPath.c_str()))
        return false;
    
    if (m_db != nullptr)
        closeDatabase();
    
    String fullPath = String("/") + String(STORAGE_TYPE) + m_dbPath;
    if (sqlite3_open(fullPath.c_str(), &m_db) != SQLITE_OK) {
        closeDatabase();
        return false;
    }

    return true;
}

bool Database::closeDatabase() {
    bool closeResult = sqlite3_close(m_db) == SQLITE_OK;
    m_db = nullptr;
    return closeResult;
}

bool Database::createDatabase(String createSQL) {
    if (!createSQL.startsWith("CREATE TABLE"))
        return false;

    if (m_db != nullptr)
        closeDatabase();

    if (m_storage->exists(m_dbPath.c_str()))
        m_storage->remove(m_dbPath.c_str());

    String fullPath = String("/") + String(STORAGE_TYPE) + m_dbPath;
    if (sqlite3_open(fullPath.c_str(), &m_db) != SQLITE_OK) {
        closeDatabase();
        return false;
    }

    return execNonQuerySQL(createSQL);
}

bool Database::execNonQuerySQL(String sql) {
    Serial.println("db_exec(" + sql + ")");
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &m_errMsg) != SQLITE_OK) {
        Serial.printf("SQL error: %s\n", m_errMsg);
        sqlite3_free(m_errMsg);
        return false;
    }
    return true;
}

//////////////////// Private methods implementation

#endif
