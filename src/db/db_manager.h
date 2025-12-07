#pragma once

#include <string>
#include <sqlite3.h>
#include <vector>
#include <tuple>

namespace filesync {

class DBManager {
public:
    DBManager(const std::string& db_path);
    ~DBManager();

    bool Init();
    bool Execute(const std::string& sql);
    
    // Metadata operations
    bool AddFile(const std::string& name, const std::string& hash, int64_t size, int64_t timestamp);
    bool GetFile(const std::string& name, std::string& hash, int64_t& size, int64_t& timestamp);
    std::vector<std::tuple<std::string, std::string, int64_t, int64_t>> GetAllFiles();
    bool AddChunk(const std::string& file_name, int32_t chunk_index, int32_t shard_index, const std::string& node_id);

private:
    std::string db_path_;
    sqlite3* db_;
};

} // namespace filesync
