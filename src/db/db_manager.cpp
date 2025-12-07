#include "db_manager.h"
#include <iostream>

namespace filesync {

DBManager::DBManager(const std::string& db_path) : db_path_(db_path), db_(nullptr) {}

DBManager::~DBManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool DBManager::Init() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const char* schema_sql = R"(
        CREATE TABLE IF NOT EXISTS files (
            name TEXT PRIMARY KEY,
            version INTEGER,
            hash TEXT,
            size INTEGER,
            is_deleted INTEGER,
            timestamp INTEGER
        );

        CREATE TABLE IF NOT EXISTS chunks (
            file_name TEXT,
            chunk_index INTEGER,
            shard_index INTEGER, -- 0 for original data if not erasure coded
            node_id TEXT,
            PRIMARY KEY (file_name, chunk_index, shard_index)
        );
    )";

    return Execute(schema_sql);
}

bool DBManager::Execute(const std::string& sql) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db_, sql.c_str(), 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}

} // namespace filesync
