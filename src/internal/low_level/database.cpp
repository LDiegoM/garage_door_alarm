#ifdef ESP32

#include <internal/low_level/database.h>

//////////////////// Constructor
Database::Database(Storage *storage, String dbPath) {
    m_storage = storage;
    m_dbPath = dbPath;
}

//////////////////// Public methods implementation
bool Database::begin() {
    if (!m_storage->exists("/db"))
        m_storage->mkdir("/db");
    return sqlite3_initialize() == SQLITE_OK;
}

String Database::getErrorMessage() {
    return m_errMsg;
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
    m_errMsg = "";
    char *errMsg;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        m_errMsg = "Database::execNonQuerySQL() error: " + String(errMsg);
        Serial.println(m_errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int Database::getValueInt(String table, String field, String where, String orderBy) {
    sqlite3_stmt *res;
    String sql = "SELECT " + field + " FROM " + table + " WHERE " + where;
    if (!orderBy.equals(""))
        sql += " ORDER BY " + orderBy;
    Serial.println("db_exec(" + sql + ")");
    m_errMsg = "";
    const char *tail;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &res, &tail) != SQLITE_OK) {
        m_errMsg = "Database::getValueInt() error: " + String(sqlite3_errmsg(m_db));
        String resp = String("Failed to fetch data: ") + m_errMsg;
        Serial.println(resp.c_str());
        return 0;
    }

    int retValue = 0;

    // This routine returns the value found at the first row, and first column.
    if (sqlite3_step(res) == SQLITE_ROW && sqlite3_column_count(res) > 0) {
        retValue = sqlite3_column_int(res, 0);
    } else {
        m_errMsg = "Database::getValueInt() error: not found";
    }

    sqlite3_finalize(res);

    return retValue;
}

float Database::getValueFloat(String table, String field, String where, String orderBy = "") {
    sqlite3_stmt *res;
    String sql = "SELECT " + field + " FROM " + table + " WHERE " + where;
    if (!orderBy.equals(""))
        sql += " ORDER BY " + orderBy;
    Serial.println("db_exec(" + sql + ")");
    m_errMsg = "";
    const char *tail;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &res, &tail) != SQLITE_OK) {
        m_errMsg = "Database::getValueInt() error: " + String(sqlite3_errmsg(m_db));
        String resp = String("Failed to fetch data: ") + m_errMsg;
        Serial.println(resp.c_str());
        return 0;
    }

    float retValue = 0;

    // This routine returns the value found at the first row, and first column.
    if (sqlite3_step(res) == SQLITE_ROW && sqlite3_column_count(res) > 0) {
        retValue = String((char *) sqlite3_column_text(res, 0)).toFloat();
    } else {
        m_errMsg = "Database::getValueFloat() error: not found";
    }

    sqlite3_finalize(res);

    return retValue;
}

String Database::getResultsAsJSON(String sql) {
    return "";
}

//////////////////// Private methods implementation

#endif
