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

bool DBManager::AddFile(const std::string& name, const std::string& hash, int64_t size, int64_t timestamp) {
    std::string sql = "INSERT OR REPLACE INTO files (name, version, hash, size, is_deleted, timestamp) VALUES ('" + 
                      name + "', 1, '" + hash + "', " + std::to_string(size) + ", 0, " + std::to_string(timestamp) + ");";
    return Execute(sql);
}

bool DBManager::GetFile(const std::string& name, std::string& hash, int64_t& size, int64_t& timestamp) {
    std::string sql = "SELECT hash, size, timestamp FROM files WHERE name = '" + name + "';";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        return false;
    }
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        size = sqlite3_column_int64(stmt, 1);
        timestamp = sqlite3_column_int64(stmt, 2);
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

std::vector<std::tuple<std::string, std::string, int64_t, int64_t>> DBManager::GetAllFiles() {
    std::vector<std::tuple<std::string, std::string, int64_t, int64_t>> files;
    std::string sql = "SELECT name, hash, size, timestamp FROM files WHERE is_deleted = 0;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        return files;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int64_t size = sqlite3_column_int64(stmt, 2);
        int64_t timestamp = sqlite3_column_int64(stmt, 3);
        files.emplace_back(name, hash, size, timestamp);
    }
    
    sqlite3_finalize(stmt);
    return files;
}

bool DBManager::AddChunk(const std::string& file_name, int32_t chunk_index, int32_t shard_index, const std::string& node_id) {
    std::string sql = "INSERT OR REPLACE INTO chunks (file_name, chunk_index, shard_index, node_id) VALUES ('" + 
                      file_name + "', " + std::to_string(chunk_index) + ", " + std::to_string(shard_index) + ", '" + node_id + "');";
    return Execute(sql);
}

} // namespace filesync
